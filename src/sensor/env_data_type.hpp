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
	LIGHT_ALS,
	LIGHT_UV,
	AIR_PM_10,
	AIR_PM_2_5,
	AIR_PM_1,
	RAIN_DISCRETE,
	RAIN_VALUE
};

struct type_info {
	const std::string unit;
	const std::string name;
	const std::string id;
};

static std::map<type, type_info> type_info_map = {
	{type::TEMPERATURE, {"°C", "temperature", "s1"}},
	{type::HUMIDITY, {"%", "humidity", "s2"}},
	{type::PRESSURE, {"hPa", "pressure"," s3"}},
	{type::AIR_QUALITY_IAQ, {" IAQ", "air quality", "s4"}},
	{type::LIGHT_ALS, {"lux", "light", "s5"}},
	{type::LIGHT_UV, {"uvi", "UV light", "s6"}},
	{type::AIR_PM_10, {"µg/m3", "PM10", "s7"}},
	{type::AIR_PM_2_5, {"µg/m3", "PM2.5", "s8"}},
	{type::AIR_PM_1, {"µg/m3", "PM1", "s9"}},
	{type::RAIN_DISCRETE, {"", "rain detected", "s10"}},
	{type::RAIN_VALUE, {"", "rain intensity", "s11"}},
};
}