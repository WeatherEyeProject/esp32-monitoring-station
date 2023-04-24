#include "pms7003.hpp"

#include "pms7003_defs.hpp"

std::string pms7003::get_name() const {
	return constant::sensor_name;
}

bool pms7003::init_sensor() {
	return true;
}

sensor_data_list pms7003::read_data() {
	sensor_data_list data;

	return data;
}

bool pms7003::suspend() {
	return true;
}

bool pms7003::wakeup() {
	return true;
}