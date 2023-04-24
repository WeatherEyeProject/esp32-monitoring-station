#pragma once

#include <map>
#include <string>

namespace environmental_data
{
enum class type {
	TEMPERATURE,
	HUMIDITY,
	PRESSURE,
	AIR_QUALITY_IAQ,
	LIGHT_LUX,
	LIGHT_UV,
	AIR_PM_10,
	AIR_PM_2_5,
	AIR_PM_1,
	RAIN_DISCRETE,
	RAIN_VALUE
};

struct type_info {
	std::string unit;
	std::string name;
	uint8_t id;
};

std::map<type, type_info> type_info_map = {
	{type::TEMPERATURE, {"°C", "temperature", 1}},
	{type::HUMIDITY, {"%", "humidity", 2}},
	{type::PRESSURE, {"Pa", "pressure", 3}},
	{type::AIR_QUALITY_IAQ, {"IAQ", "air quality", 4}},
	{type::LIGHT_LUX, {"lux", "light", 5}},
	{type::LIGHT_UV, {"uvi", "UV light", 6}},
	{type::AIR_PM_10, {"µg/m3", "particulate matter", 7}},
	{type::AIR_PM_2_5, {"µg/m3", "particulate matter", 8}},
	{type::AIR_PM_1, {"µg/m3", "particulate matter", 9}},
	{type::RAIN_DISCRETE, {"", "rain detected", 10}},
	{type::RAIN_VALUE, {"??", "rain intensity", 11}},
};
}