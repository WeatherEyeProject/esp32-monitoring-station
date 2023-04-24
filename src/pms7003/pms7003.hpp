#pragma once

#include <vector>

#include <sensor/sensor.hpp>

class pms7003 : sensor
{
public:
	bool init_sensor() override;
	bool wakeup() override;
	bool suspend() override;
	std::string get_name() const override;
	sensor_data_list read_data() override;

private:

};