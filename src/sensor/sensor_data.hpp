#pragma once

#include <list>

#include "env_data_type.hpp"

class sensor_data {
	environmental_data::type type;
	float data;

public:
	sensor_data(environmental_data::type type, float data = 0.0) :
		type(type), data(data)
	{ }

	void set_value(float value)
	{
		data = value;
	}

	float get_value() const
	{
		return data;
	}

	const std::string& get_name() const
	{
		return environmental_data::type_info_map.at(type).name;
	}

	const std::string& get_unit() const
	{
		return environmental_data::type_info_map.at(type).unit;
	}

	std::string get_id() const
	{
		return environmental_data::type_info_map.at(type).id;
	}
};

using sensor_data_list = std::list<sensor_data>;