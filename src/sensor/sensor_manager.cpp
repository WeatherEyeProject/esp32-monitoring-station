#include "sensor_manager.hpp"

#include <bme680/bme680.hpp>
#include <ltr390/ltr390.hpp>
#include <pms7003/pms7003.hpp>

#include <log.h>

sensor_manager::sensor_manager()
{
	sensor_list.push_back(std::make_unique<bme680>());
	sensor_list.push_back(std::make_unique<ltr390>());
	sensor_list.push_back(std::make_unique<pms7003>());
}

bool sensor_manager::init_sensors()
{
	for (auto& sensor : sensor_list) {
		auto ret = sensor->init_sensor();
		if (!ret) {
			return false;
		}
	}

	return true;
}

bool sensor_manager::wakeup_sensors()
{
	for (auto& sensor : sensor_list) {
		auto ret = sensor->wakeup();
		if (!ret) {
			return false;
		}
	}

	return true;
}

bool sensor_manager::suspend_sensors()
{
	for (auto& sensor : sensor_list) {
		auto ret = sensor->suspend();
		if (!ret) {
			return false;
		}
	}

	return true;
}

sensor_data_list sensor_manager::gather_data()
{
	sensor_data_list data;
	for (auto& sensor : sensor_list) {
		data.splice(data.end(), sensor->read_data());
	}

	return data;
}