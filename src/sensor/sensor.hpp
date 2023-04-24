#pragma once

#include <string>

#include <sensor/sensor_data.hpp>

class sensor {
public:
	virtual ~sensor() { }

	virtual bool init_sensor() = 0;

	virtual bool wakeup() = 0;
	virtual bool suspend() = 0;

	virtual std::string get_name() const = 0;

	virtual sensor_data_list read_data() = 0;
};