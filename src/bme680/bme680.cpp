#include "bme680.hpp"

#include "sensor_api_glue.h"

#include <iostream>

namespace constant
{
const std::string sensor_name("BME680");
}

bme680::bme680()
	: i2c_dev(sensors_i2c_bus::get_instance()),
	sensor_ready(false)
{
	i2c_dev->init_bus();

	dev.read = &bme68x_i2c_read;
	dev.write = &bme68x_i2c_write;
	dev.delay_us = &bme68x_delay_us;

	dev.intf = BME68X_I2C_INTF;
	dev.intf_ptr = (void*)(BME68X_I2C_ADDR_LOW << 1);
}

bme680::~bme680()
{
	suspend();
}

std::string bme680::get_name() const
{
	return constant::sensor_name;
}

bool bme680::init_sensor()
{
	auto ret = bme68x_selftest_check(&dev);
	if (ret != 0)
		return false;

	ret = bme68x_init(&dev);
	if (ret != 0)
		return false;

	return true;
}

sensor_data_list bme680::read_data()
{
	sensor_data_list data;

	bme68x_data bme_data;
	uint8_t inst;
	bme68x_get_data(BME68X_FORCED_MODE, &bme_data, &inst, &dev);

	std::cout << "data " << bme_data.temperature << " " << bme_data.pressure << " " << bme_data.humidity << " " << bme_data.gas_resistance << std::endl;

	return data;
}

bool bme680::suspend()
{
	if (sensor_suspended)
		return true;



	sensor_suspended = true;

	return true;
}

bool bme680::wakeup()
{
	if (!sensor_suspended)
		return true;



	sensor_suspended = false;

	return true;
}