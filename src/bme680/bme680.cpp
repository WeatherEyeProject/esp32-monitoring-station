#include "bme680.hpp"

#include <iostream>
#include <cmath>

#include "sensor_api_glue.h"

namespace constant
{
const std::string sensor_name("BME680");
const uint8_t sensor_addr = BME68X_I2C_ADDR_LOW << 1;
}

bme680::bme680()
	: i2c_dev(sensors_i2c_bus::get_instance()),
	sensor_ready(false)
{
	i2c_dev->init_bus();
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
	if (!do_sensor_selfcheck())
		return false;

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
	sensor_data_list data;

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