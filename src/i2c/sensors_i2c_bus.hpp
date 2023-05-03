#pragma once

#include <memory>
#include <vector>
#include <optional>

class sensors_i2c_bus;
using sensors_i2c_bus_ptr = std::shared_ptr<sensors_i2c_bus>;

class sensors_i2c_bus {
	sensors_i2c_bus();
	~sensors_i2c_bus();

	static sensors_i2c_bus_ptr instance;

	bool bus_ready;

public:
	bool init_bus();
	static sensors_i2c_bus_ptr get_instance();

	std::optional<uint8_t> i2c_read(const uint8_t dev_addr, const uint8_t mem_addr);
	std::optional<std::vector<uint8_t>> i2c_read_bytes(const uint8_t dev_addr, const uint8_t mem_addr, const uint32_t bytes);
	bool i2c_write(const uint8_t dev_addr, const uint8_t mem_addr, const uint8_t data);
	bool i2c_write_bytes(const uint8_t dev_addr, const uint8_t mem_addr, const uint8_t* data, const uint32_t bytes);
};