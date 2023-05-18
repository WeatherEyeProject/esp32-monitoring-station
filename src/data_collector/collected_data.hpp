#pragma once

#include <sensor/sensor_data.hpp>
#include <time.h>

struct data_packet {
	time_t time;
	sensor_data_list data;
};

using collected_data_list = std::list<data_packet>;