#pragma once

#include <bme68x.h>
#include <bsec.hpp>
#include <sensor/sensor.hpp>
#include <i2c/sensors_i2c_bus.hpp>
#include <thread>
#include <mutex>

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
	bool enable_learning_func;

	Bsec bsec;

	std::thread learning_thread;
	std::mutex sensor_meas_mtx;

	bool do_sensor_selfcheck();
	bool check_sensor_status();
	void learning_func();
};