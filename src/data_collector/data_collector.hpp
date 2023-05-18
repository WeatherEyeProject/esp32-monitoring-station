#pragma once

#include <vector>

#include "collected_data.hpp"

class data_collector {
public:
	void add_new_data(sensor_data_list data);

	bool is_empty();
	void trash_last_packet();
	std::vector<char> get_json_data_packet(const size_t max_count);

private:
	collected_data_list collected_data;
	collected_data_list tmp_packet;
};