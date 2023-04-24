#pragma once

#include <vector>

#include <sensor/sensor.hpp>
#include <i2c/sensors_i2c_bus.hpp>

class ltr390 : sensor
{
public:
	ltr390();
	
	bool init_sensor() override;
	bool wakeup() override;
	bool suspend() override;
	std::string get_name() const override;
	sensor_data_list read_data() override;

private:
	sensors_i2c_bus_ptr i2c_dev;
	
	bool i2c_write(uint8_t reg, uint8_t data);
	std::vector<uint8_t> i2c_read(uint8_t reg, uint32_t bytes);
	uint8_t i2c_read(uint8_t reg);
};