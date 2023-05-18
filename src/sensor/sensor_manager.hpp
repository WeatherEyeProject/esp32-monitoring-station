#pragma once

#include <memory>
#include <vector>

#include "sensor.hpp"

class sensor_manager {
public:
	sensor_manager();

	bool init_sensors();

	bool wakeup_sensors();
	bool suspend_sensors();

	sensor_data_list gather_data();

private:
	std::vector<std::unique_ptr<sensor>> sensor_list;
};