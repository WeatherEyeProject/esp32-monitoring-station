#pragma once

#include <bme68x.h>
#include <sensor/sensor.hpp>
#include <i2c/sensors_i2c_bus.hpp>

class bme680 : sensor {
public:
    bme680();
	~bme680();

	bool init_sensor() override;
	bool wakeup() override;
	bool suspend() override;
	std::string get_name() const override;
	sensor_data_list read_data() override;

private:
	sensors_i2c_bus_ptr i2c_dev;
	bool sensor_ready;
	bool sensor_suspended;

	bool do_sensor_selfcheck();
};