#include "ltr390.hpp"

#include <driver/i2c.h>
#include <sensor/env_data_type.hpp>

#include "ltr390_defs.hpp"

#include <iostream>

ltr390::ltr390() : i2c_dev(sensors_i2c_bus::get_instance())
{ }

std::string ltr390::get_name() const
{
	return constant::sensor_name;
}

bool ltr390::init_sensor()
{
	auto part_id = i2c_read(constant::address::part_id);

	if (part_id != constant::sensor_part_id) {
		std::cout << "Part id read error, got: " << std::hex << int(part_id) << std::endl;
		return false;
	}

	/*i2c_write(constant::address::meas_rate, 0x04);
	i2c_write(constant::address::main_ctrl, 0x02);
	i2c_write(constant::address::gain, 0x04);*/

	return true;
}

bool ltr390::wakeup()
{
	return true;
}

bool ltr390::suspend()
{
	return true;
}

sensor_data_list ltr390::read_data()
{
	sensor_data_list data;

	/*registers::main_ctrl ctl;
	ctl.enable = constant::operation::enable;
	ctl.mode = constant::modes::als;


	std::cout << "ctl: " << std::hex << int(ctl) << std::endl;
	auto part_id = i2c_read(constant::address::main_ctrl);
	std::cout << "got: " << std::hex << int(part_id) << std::endl;
	if (part_id == ctl) {
		std::cout << "ok" << std::endl;
	}
	sleep(1);

	light_data light;
	light.lsb = i2c_read(constant::address::als_data_start);
	light.msb = i2c_read(constant::address::als_data_start+1);
	light.hsb = i2c_read(constant::address::als_data_end);

	uint32_t out = light;
	std::cout << "LTR ALS read was: " << std::dec << std::to_string(light.lsb | (light.msb << 2) | (light.hsb << 4)) << std::endl;*/

	return data;
}

std::vector<uint8_t> ltr390::i2c_read(uint8_t reg, uint32_t bytes)
{
	return i2c_dev->i2c_read_bytes(constant::i2c_address, reg, bytes);
}

bool ltr390::i2c_write(uint8_t reg, uint8_t data)
{
	return i2c_dev->i2c_write_byte(constant::i2c_address, reg, data);
}

uint8_t ltr390::i2c_read(uint8_t reg)
{
	return i2c_dev->i2c_read(constant::i2c_address, reg);
}