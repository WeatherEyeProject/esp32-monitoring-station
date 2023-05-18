#pragma once

#include <vector>

#include <cJSON.h>

#include <data_collector/collected_data.hpp>

class json_converter {
public:
	static std::vector<char> convert_data_to_json(collected_data_list data);

private:
	static void create_timestamp(const time_t* time, char* out);
	static void add_timestamp_fragment(cJSON* obj, const time_t* time);
	static void add_data_fragment(cJSON* obj, const sensor_data_list& data);
};