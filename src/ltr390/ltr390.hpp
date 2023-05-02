#pragma once

#include <vector>

#include <sensor/sensor.hpp>
#include <i2c/sensors_i2c_bus.hpp>

#include "ltr390_defs.hpp"

class ltr390: sensor
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
	bool sensor_ready;
	bool sensor_suspended;

	uint8_t sensor_gain;
	uint8_t sensor_resolution;

	bool i2c_write(const uint8_t reg, const uint8_t data);
	std::optional<std::vector<uint8_t>> i2c_read(const uint8_t reg, const uint32_t bytes);
	std::optional<uint8_t> i2c_read(const uint8_t reg);
	bool wait_for_new_data();
	std::optional<uint32_t> get_meas_from_sensor(const uint8_t sensor_mode, const uint8_t first_data_reg);
	float calculate_lux_from_raw(uint32_t raw);
	float calculate_uvi_from_raw(uint32_t raw);
	bool set_sensor_gain(uint8_t gain);
	std::optional<float> get_calculated_lux_from_sensor();
	std::optional<float> get_calculated_uvi_from_sensor();
};