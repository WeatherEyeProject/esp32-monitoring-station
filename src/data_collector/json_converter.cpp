#include "json_converter.hpp"

#include <cstring>

namespace constant
{
const char timestamp_format[] = "%Y-%m-%dT%H:%M:%SZ";
const uint16_t timestamp_size = sizeof(timestamp_format) + 2; // year has 4 characters
const char timestamp_name[] = "dateTime";
}

std::vector<char> json_converter::convert_data_to_json(collected_data_list data)
{
	auto json_data = cJSON_CreateArray();

	for (const auto& item : data) {
		auto element = cJSON_CreateObject();

		add_timestamp_fragment(element, &item.time);
		add_data_fragment(element, item.data);

		cJSON_AddItemToArray(json_data, element);
	}

	auto json_raw = cJSON_PrintUnformatted(json_data);
	cJSON_free(json_data);

	auto json_len = strlen(json_raw);
	std::vector<char> out(json_len);
	memcpy(out.data(), json_raw, json_len);

	free(json_raw);
	
	return out;
}

void json_converter::create_timestamp(const time_t* time, char* out) {
	tm* utc_time = gmtime(time);
	strftime(out, constant::timestamp_size, constant::timestamp_format,
			 utc_time);
}

void json_converter::add_timestamp_fragment(cJSON* obj, const time_t* time) {
	char time_arr[constant::timestamp_size];
	create_timestamp(time, time_arr);
	cJSON_AddStringToObject(obj, constant::timestamp_name, time_arr);
}

void json_converter::add_data_fragment(cJSON* obj, const sensor_data_list& data) {
	for (const auto& sensor : data) {
		char sensor_val[20];
		sprintf(sensor_val, "%.2f", sensor.get_value());
		cJSON_AddRawToObject(obj, sensor.get_id().c_str(), sensor_val);
	}
}