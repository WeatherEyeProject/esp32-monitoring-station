#include "sensors_i2c_bus.hpp"

#include <driver/i2c.h>

#include <iostream>

namespace constant
{
const uint8_t sensors_i2c_sda_gpio = 21;
const uint8_t sensors_i2c_scl_gpio = 22;
const uint8_t i2c_port_num = 0;
const unsigned ticks_timeout = 100;
const uint32_t i2c_speed = 100'000;

const i2c_config_t i2c_conf = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = sensors_i2c_sda_gpio,
	.scl_io_num = sensors_i2c_scl_gpio,
	.sda_pullup_en = GPIO_PULLUP_ENABLE,
	.scl_pullup_en = GPIO_PULLUP_ENABLE,
	.master = {.clk_speed = i2c_speed},
	.clk_flags = 0,
};
}

sensors_i2c_bus_ptr sensors_i2c_bus::instance;

sensors_i2c_bus_ptr sensors_i2c_bus::get_instance()
{
	if (!instance) {
		class public_cstor : public sensors_i2c_bus {};
		instance = std::make_shared<public_cstor>();
	}

	return instance;
}

sensors_i2c_bus::sensors_i2c_bus()
	: bus_ready(false)
{ }

sensors_i2c_bus::~sensors_i2c_bus()
{
	i2c_driver_delete(constant::i2c_port_num);
}

bool sensors_i2c_bus::init_bus()
{
	if (bus_ready)
		return true;

	bool result = i2c_param_config(constant::i2c_port_num, &constant::i2c_conf);
	if (result != ESP_OK) {
		std::cout << "I2C param config error!" << std::endl;
		return false;
	}

	result = i2c_driver_install(constant::i2c_port_num, I2C_MODE_MASTER, 0, 0, 0);
	if (result != ESP_OK) {
		std::cout << "I2C driver install error!" << std::endl;
		return false;
	}

	bus_ready = true;

	return true;
}

bool sensors_i2c_bus::i2c_read_common(const uint8_t dev_addr, const uint8_t reg_addr, uint8_t* data, const uint32_t bytes)
{
	if (!bus_ready) {
		return false;
	}

	auto cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, dev_addr, true);
	i2c_master_write_byte(cmd_handle, reg_addr, true);
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, dev_addr | 0x01, true);
	i2c_master_read(cmd_handle, data, bytes, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd_handle);
	auto esp_rslt = i2c_master_cmd_begin(constant::i2c_port_num, cmd_handle, constant::ticks_timeout);
	i2c_cmd_link_delete(cmd_handle);

	auto ret = esp_rslt == ESP_OK;
	if (!ret) {
		std::cout << "I2C read error: " << std::to_string(esp_rslt) << std::endl;
	}

	return ret;
}

bool sensors_i2c_bus::i2c_write_common(const uint8_t dev_addr, const uint8_t reg_addr, const uint8_t* data, const uint32_t bytes)
{
	if (!bus_ready) {
		return false;
	}

	auto cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, dev_addr, true);
	i2c_master_write_byte(cmd_handle, reg_addr, true);
	i2c_master_write(cmd_handle, data, bytes, true);
	i2c_master_stop(cmd_handle);
	auto esp_rslt = i2c_master_cmd_begin(constant::i2c_port_num, cmd_handle, constant::ticks_timeout);
	i2c_cmd_link_delete(cmd_handle);

	auto ret = esp_rslt == ESP_OK;
	if (!ret) {
		std::cout << "I2C write error: " << std::to_string(esp_rslt) << std::endl;
	}

	return ret;
}

std::optional<uint8_t> sensors_i2c_bus::i2c_read(const uint8_t dev_addr, const uint8_t reg_addr)
{
	uint8_t data;
	auto ret = i2c_read_common(dev_addr, reg_addr, &data, 1);

	if (!ret)
		return std::nullopt;

	return data;
}

std::optional<std::vector<uint8_t>> sensors_i2c_bus::i2c_read_bytes(const uint8_t dev_addr, const uint8_t reg_addr, const uint32_t bytes)
{
	std::vector<uint8_t> data(bytes);
	auto ret = i2c_read_common(dev_addr, reg_addr, data.data(), bytes);

	if (!ret)
		return std::nullopt;

	return data;
}

bool sensors_i2c_bus::i2c_read_bytes(const uint8_t dev_addr, const uint8_t reg_addr, uint8_t* data, const uint32_t bytes)
{
	return i2c_read_common(dev_addr, reg_addr, data, bytes);
}

bool sensors_i2c_bus::i2c_write(const uint8_t dev_addr, const uint8_t reg_addr, const uint8_t data)
{
	return i2c_write_common(dev_addr, reg_addr, &data, 1);
}

bool sensors_i2c_bus::i2c_write_bytes(const uint8_t dev_addr, const uint8_t reg_addr, const uint8_t* data, const uint32_t bytes)
{
	return i2c_write_common(dev_addr, reg_addr, data, bytes);
}