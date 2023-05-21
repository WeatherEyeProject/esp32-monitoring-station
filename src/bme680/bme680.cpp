#include "bme680.hpp"

#include <iostream>
#include <cmath>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <config/bsec_configs.hpp>
#include <log.h>

#include "sensor_api_glue.h"

// Enable for production
#define DO_SENSOR_SELFCHECK 0

namespace constant
{
const std::string sensor_name("BME680");
const uint8_t sensor_addr = BME68X_I2C_ADDR_LOW << 1;
const uint8_t iaq_medium_accuracy = 2;
const std::chrono::seconds fast_sampling_interval(3);
const std::chrono::seconds slow_sampling_interval(30);
const uint32_t learning_state_save_interval_ms = 3 * 24 * 60 * 60 * 1000; // 3 days

const bsec_virtual_sensor_t sensor_list[] = {
	BSEC_OUTPUT_IAQ,
	BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
	BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
	BSEC_OUTPUT_RAW_PRESSURE,
	BSEC_OUTPUT_RUN_IN_STATUS
};

const size_t sensor_list_size = sizeof(sensor_list) / sizeof(bsec_virtual_sensor_t);
}

bme680::bme680()
	: i2c_dev(sensors_i2c_bus::get_instance()),
	sensor_ready(false),
	enable_learning_func(true),
	next_learning_state_save(0)
{
	i2c_dev->init_bus();
}

bme680::~bme680()
{
	{
		std::lock_guard lock(sensor_meas_mtx);
		enable_learning_func = false;
	}
	learning_thread.join();
}

std::string bme680::get_name() const
{
	return constant::sensor_name;
}

void bme680::learning_func()
{
	std::unique_lock lock(sensor_meas_mtx);

#if DO_SENSOR_SELFCHECK
	// Sensor gets warm after selfcheck, wait a bit
	std::this_thread::sleep_for(constant::slow_sampling_interval);
#endif

	next_learning_state_save = bsec.getTimeMs() + constant::learning_state_save_interval_ms;

	while (enable_learning_func) {
		bsec.run();

		lock.unlock();
		bool enable_fast_sampling = !bsec.runInStatus || !sensor_suspended ||
			bsec.iaqAccuracy < constant::iaq_medium_accuracy;
		std::this_thread::sleep_for(enable_fast_sampling ?
									constant::fast_sampling_interval : constant::slow_sampling_interval);

		if (bsec.iaqAccuracy >= constant::iaq_medium_accuracy &&
			bsec.getTimeMs() > next_learning_state_save) {
			std::vector<uint8_t> data(BSEC_MAX_STATE_BLOB_SIZE);

			bsec.getState(data.data());

			logi(__func__, "Saving bsec learning state to NVS");
			if (!bsec_storage.save_learned_state(data)) {
				loge(__func__, "Saving NVS error, reinit storage");
				bsec_storage.erase_partition();
			}

			next_learning_state_save = bsec.getTimeMs() + constant::learning_state_save_interval_ms;
		}
		lock.lock();
	}
}

bool bme680::init_sensor()
{
#if DO_SENSOR_SELFCHECK
	if (!do_sensor_selfcheck())
		return false;
#endif

	bsec.begin(BME68X_I2C_INTF,
			   bme68x_i2c_read,
			   bme68x_i2c_write,
			   bme68x_delay_us,
			   (void*)constant::sensor_addr);
	auto ret = check_sensor_status();
	if (!ret)
		return false;

	bsec.setConfig(bsec_config_33v_3s_4d);
	ret = check_sensor_status();
	if (!ret)
		return false;

	bsec.updateSubscription(constant::sensor_list, constant::sensor_list_size, BSEC_SAMPLE_RATE_LP);
	ret = check_sensor_status();
	if (!ret)
		return false;

	if (!bsec_storage.init_partition()) {
		return false;
	}

	if (bsec_storage.get_state_version() != 0) {
		auto data = bsec_storage.get_learned_state();
		if (data.size() == 0) {
			logi(__func__, "Empty learned state read, do erase");
			if (!bsec_storage.erase_partition()) {
				return false;
			}
		}
		else {
			logi(__func__, "Loading learned stare, rev: %d", bsec_storage.get_state_version());
			bsec.setState(data.data());
		}
	}

	sensor_ready = true;
	sensor_suspended = true;

	learning_thread = std::thread([&] {
		learning_func();
	});

	return true;
}

bool bme680::do_sensor_selfcheck()
{
	bme68x_dev dev;

	dev.read = &bme68x_i2c_read;
	dev.write = &bme68x_i2c_write;
	dev.delay_us = &bme68x_delay_us;

	dev.intf = BME68X_I2C_INTF;
	dev.intf_ptr = (void*)constant::sensor_addr;

	return bme68x_selftest_check(&dev) == 0;
}

sensor_data_list bme680::read_data()
{
	if (!sensor_ready || sensor_suspended)
		return {};

	std::lock_guard lock(sensor_meas_mtx);
	// Always wait for fresh data
	auto cur_time = bsec.getTimeMs();
	while (bsec.nextCall > cur_time) {
		bsec.delay_us((bsec.nextCall - cur_time) * 1000, nullptr);
		cur_time = bsec.getTimeMs();
	}
	auto run = bsec.run();
	if (!run) {
		std::cout << "Run failed, why?" << std::endl;
	}

	if (!check_sensor_status()) {
		return {};
	}

	sensor_data_list data;

	if (bsec.iaqAccuracy >= constant::iaq_medium_accuracy) {
		data.push_back(::sensor_data(environmental_data::type::AIR_QUALITY_IAQ, std::round(bsec.iaq)));
	}
	else {
		std::cout << "IAQ accuracy too low: " << std::to_string(bsec.iaqAccuracy) << std::endl;
		std::cout << "Not adding IAQ to results: " << std::to_string(bsec.iaq) << std::endl;
	}
	data.push_back(::sensor_data(environmental_data::type::TEMPERATURE, std::round(bsec.temperature * 10.0) / 10.0));
	data.push_back(::sensor_data(environmental_data::type::HUMIDITY, std::round(bsec.humidity)));
	data.push_back(::sensor_data(environmental_data::type::PRESSURE, std::round(bsec.pressure / 100.0)));

	return data;
}

bool bme680::suspend()
{
	if (!sensor_ready)
		return false;

	if (sensor_suspended)
		return true;

	std::lock_guard lock(sensor_meas_mtx);
	sensor_suspended = true;

	return true;
}

bool bme680::wakeup()
{
	if (!sensor_ready)
		return false;

	if (!sensor_suspended)
		return true;

	std::lock_guard lock(sensor_meas_mtx);
	sensor_suspended = false;

	return true;
}

bool bme680::check_sensor_status()
{
	if (bsec.bsecStatus != BME68X_OK) {
		std::cout << "bsec error status: " << std::to_string(int(bsec.bsecStatus)) << std::endl;
	}
	if (bsec.bme68xStatus != BME68X_OK) {
		std::cout << "bme680 error status: " << std::to_string(bsec.bme68xStatus) << std::endl;
	}
	return bsec.bsecStatus == BME68X_OK && bsec.bme68xStatus == BME68X_OK;
}