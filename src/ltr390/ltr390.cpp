#include "ltr390.hpp"

#include <driver/i2c.h>
#include <sensor/env_data_type.hpp>

#include "ltr390_defs.hpp"

#include <iostream>
#include <cmath>

using namespace ltr390_internal;

ltr390::ltr390()
	: i2c_dev(sensors_i2c_bus::get_instance()),
	sensor_ready(false)
{
	i2c_dev->init_bus();
}

ltr390::~ltr390()
{
	suspend();
}

std::string ltr390::get_name() const
{
	return constant::sensor_name;
}

bool ltr390::init_sensor()
{
	if (sensor_ready)
		return true;

	registers::main_ctrl reg = { .raw = 0 };

	reg.sw_reset = true;
	auto ret = i2c_write(constant::address::main_ctrl, reg.raw);
	if (!ret) {
		return false;
	}

	auto part_id = i2c_read(constant::address::part_id);
	if (part_id && *part_id != constant::sensor_part_id) {
		std::cout << "Part id read error, got: " << std::hex << int(*part_id) << std::endl;
		return false;
	}

	// Don't care about meas time, use highest resolution
	registers::meas_rate rate = { .raw = 0 };
	rate.resolution = constant::resolution::bit_20;
	rate.rate = constant::rate::ms_500;
	ret = i2c_write(constant::address::meas_rate, rate.raw);
	if (!ret) {
		return false;
	}

	sensor_resolution = constant::resolution::bit_20;

	sensor_ready = true;
	sensor_suspended = true;

	return true;
}

bool ltr390::wakeup()
{
	if (!sensor_suspended)
		return true;

	registers::main_ctrl reg = { .raw = 0 };
	reg.enable = constant::operation::enable;

	auto ret = i2c_write(constant::address::main_ctrl, reg.raw);
	if (!ret) {
		return false;
	}

	sensor_suspended = false;

	return true;
}

bool ltr390::suspend()
{
	if (sensor_suspended)
		return true;

	registers::main_ctrl reg = { .raw = 0 };
	reg.enable = constant::operation::disable;

	auto ret = i2c_write(constant::address::main_ctrl, reg.raw);
	if (!ret) {
		return false;
	}

	sensor_suspended = true;

	return true;
}

std::optional<uint32_t> ltr390::get_meas_from_sensor(const uint8_t sensor_mode, const uint8_t first_data_reg)
{
	registers::main_ctrl ctl = { .raw = 0 };
	ctl.enable = true;
	ctl.mode = sensor_mode;
	auto ret = i2c_write(constant::address::main_ctrl, ctl.raw);
	if (!ret) {
		return ret;
	}

	ret = wait_for_new_data();
	if (!ret) {
		return ret;
	}

	// Data is stored in three registers from LSB to MSB
	// Shift data to uin32_t starting from 3'rd byte
	uint32_t data = 0;
	for (uint8_t reg = first_data_reg; reg < first_data_reg + 3; reg++) {
		auto reg_data = i2c_read(reg);
		if (!reg_data)
			return std::nullopt;
		data >>= 8;
		data |= *reg_data << 16;
	}

	return data;
}

sensor_data_list ltr390::read_data()
{
	if (!sensor_ready || sensor_suspended)
		return {};

	auto als = get_calculated_lux_from_sensor();
	if (!als) {
		return {};
	}

	auto uvs = get_calculated_uvi_from_sensor();
	if (!uvs) {
		return {};
	}

	sensor_data_list data;
	data.push_back(sensor_data(environmental_data::type::LIGHT_ALS, *als));
	data.push_back(sensor_data(environmental_data::type::LIGHT_UV, *uvs));
	return data;
}

bool ltr390::wait_for_new_data()
{
	auto wait_func = [this](uint16_t max_tries, uint16_t sleep_ms, uint8_t expected_status) {
		uint16_t tries = 0;
		while (tries++ != max_tries) {
			auto data = i2c_read(constant::address::status);
			if (!data)
				return false;

			if (((registers::main_status)*data).data_status == expected_status)
				return true;

			vTaskDelay(pdMS_TO_TICKS(sleep_ms));
		}

		std::cout << "Waiting for new data timeout" << std::endl;
		return false;
	};

	// At first, wait for measurements to start -> data_status should be 0
	// This usually takes 5-10ms, use 20ms for safe margin
	const uint16_t meas_start_sleep_ms = 5;
	const uint16_t max_meas_start_tries = 20 / meas_start_sleep_ms;

	auto ret = wait_func(max_meas_start_tries, meas_start_sleep_ms, 0);
	if (!ret) {
		return ret;
	}

	// 25ms is minimal measurement time
	const uint32_t data_ready_sleep_ms = 25;
	// 2000 ms is max sampling time
	const uint16_t max_data_ready_tries = 2000 / data_ready_sleep_ms;

	ret = wait_func(max_data_ready_tries, data_ready_sleep_ms, 1);

	return ret;
}

bool ltr390::set_sensor_gain(uint8_t new_gain)
{
	registers::gain_range gain_reg = { .raw = 0 };
	gain_reg.gain = new_gain;
	auto ret = i2c_write(constant::address::gain, gain_reg.raw);

	if (ret)
		sensor_gain = new_gain;

	return ret;
}

std::optional<float> ltr390::get_calculated_lux_from_sensor()
{
	std::optional<uint32_t> als;

	for (uint8_t cur_gain = constant::gain::range_18; cur_gain >= constant::gain::range_1; cur_gain--) {
		set_sensor_gain(cur_gain);

		als = get_meas_from_sensor(constant::modes::als, constant::address::als_data_lsb);
		if (!als) {
			return std::nullopt;
		}

		if (*als < 0xFFF00) {
			break;
		}

		std::cout << "Sensor out of range, lowering gain..." << std::endl;
	}

	auto lux = calculate_lux_from_raw(*als);

	return std::round(lux * 10) / 10;
}

std::optional<float> ltr390::get_calculated_uvi_from_sensor()
{
	if (!set_sensor_gain(constant::gain::range_18)) {
		return std::nullopt;
	}

	auto raw = get_meas_from_sensor(constant::modes::uvs, constant::address::uvs_data_lsb);
	if (!raw) {
		return std::nullopt;
	}

	auto uvi = calculate_uvi_from_raw(*raw);

	return std::round(uvi);
}

float ltr390::calculate_lux_from_raw(uint32_t raw)
{
	return (0.6 * (float)raw) / (constant::gain_table[sensor_gain] *
								 constant::resolution_table[sensor_resolution]);
}

float ltr390::calculate_uvi_from_raw(uint32_t raw)
{
	return (float)raw / constant::uvi_sensitivity;
}

std::optional<std::vector<uint8_t>> ltr390::i2c_read(const uint8_t reg, const uint32_t bytes)
{
	return i2c_dev->i2c_read_bytes(constant::i2c_address, reg, bytes);
}

bool ltr390::i2c_write(const uint8_t reg, const uint8_t data)
{
	return i2c_dev->i2c_write_byte(constant::i2c_address, reg, data);
}

std::optional<uint8_t> ltr390::i2c_read(const uint8_t reg)
{
	return i2c_dev->i2c_read(constant::i2c_address, reg);
}