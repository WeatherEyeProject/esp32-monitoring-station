#pragma once

#include <memory>
#include <vector>

class sensors_i2c_bus;
using sensors_i2c_bus_ptr = std::shared_ptr<sensors_i2c_bus>;

class sensors_i2c_bus {
	sensors_i2c_bus();
	~sensors_i2c_bus();

	static sensors_i2c_bus_ptr instance;

	bool bus_ready = false;

public:
	bool init_bus();
	static sensors_i2c_bus_ptr get_instance();

	uint8_t i2c_read(const uint8_t dev_addr, const uint8_t mem_addr);
	std::vector<uint8_t> i2c_read_bytes(const uint8_t dev_addr, const uint8_t mem_addr, uint32_t bytes);
	bool i2c_write_byte(const uint8_t dev_addr, const uint8_t mem_addr, const uint8_t data);
};