#pragma once

#include <string>

namespace constant
{
const std::string sensor_name("LTR390");
const uint8_t i2c_address = 0xA6;
const uint8_t sensor_part_id = 0xB2;

namespace address
{
const uint8_t main_ctrl = 0x00;
const uint8_t meas_rate = 0x04;
const uint8_t gain = 0x05;
const uint8_t part_id = 0x06;
const uint8_t status = 0x07;
const uint8_t als_data_start = 0x0D;
const uint8_t als_data_end = 0x0F;
const uint8_t uvs_data_start = 0x10;
const uint8_t uvs_data_end = 0x12;
}

namespace modes
{
const uint8_t uvs = 1;
const uint8_t als = 0;
}

namespace operation
{
const uint8_t enable = 1;
const uint8_t disable = 0;
}
}

namespace registers
{
union main_ctrl {
	struct {
		const uint8_t reserved1 : 3;
		uint8_t sw_reset : 1;
		uint8_t mode : 1;
		const uint8_t reserved2 : 1;
		uint8_t enable : 1;
		const uint8_t reserved3 : 1;
	} __packed;

	uint8_t raw;
};

union meas_rate {
	struct {
		const uint8_t reserved1 : 1;
		uint8_t resolution : 3;
		const uint8_t reserved2 : 1;
		uint8_t rate : 3;
	} __packed;

	uint8_t raw;
};

union light_data {
	struct {
		const uint8_t unused;
		uint8_t hsb;
		uint8_t msb;
		uint8_t lsb;
	} __packed;

	uint32_t value;
};
}