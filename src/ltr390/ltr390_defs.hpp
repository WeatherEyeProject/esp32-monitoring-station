#pragma once

#include <string>

namespace ltr390_internal
{
namespace constant
{
const std::string sensor_name("LTR390");
const uint8_t i2c_address = 0xA6;
const uint8_t sensor_part_id = 0xB2;
const float uvi_sensitivity = 2300;

namespace address
{
const uint8_t main_ctrl = 0x00;
const uint8_t meas_rate = 0x04;
const uint8_t gain = 0x05;
const uint8_t part_id = 0x06;
const uint8_t status = 0x07;
const uint8_t als_data_lsb = 0x0D;
const uint8_t als_data_msb = 0x0E;
const uint8_t als_data_hsb = 0x0F;
const uint8_t uvs_data_lsb = 0x10;
const uint8_t uvs_data_msb = 0x11;
const uint8_t uvs_data_hsb = 0x12;
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

namespace resolution
{
const uint8_t bit_20 = 0x0;
const uint8_t bit_19 = 0x1;
const uint8_t bit_18 = 0x2;
const uint8_t bit_17 = 0x3;
const uint8_t bit_16 = 0x4;
const uint8_t bit_13 = 0x5;
}

namespace rate
{
const uint8_t ms_25 = 0x0;
const uint8_t ms_50 = 0x1;
const uint8_t ms_100 = 0x2;
const uint8_t ms_200 = 0x3;
const uint8_t ms_500 = 0x4;
const uint8_t ms_1000 = 0x5;
const uint8_t ms_2000 = 0x6;
}

namespace gain
{
const uint8_t range_1 = 0x0;
const uint8_t range_3 = 0x1;
const uint8_t range_6 = 0x2;
const uint8_t range_9 = 0x3;
const uint8_t range_18 = 0x4;
}

const float gain_table[] = { 1, 3, 6, 9, 18 };
const float resolution_table[] = { 4, 2, 1, 0.5, 0.25, 0.125 };
}

namespace registers
{
union main_ctrl {
	struct {
		const uint8_t reserved3 : 1;
		uint8_t enable : 1;
		const uint8_t reserved2 : 1;
		uint8_t mode : 1;
		uint8_t sw_reset : 1;
		const uint8_t reserved1 : 3;
	} __packed;

	uint8_t raw;
};

union main_status {
	struct {
		const uint8_t reserved2 : 3;
		uint8_t data_status : 1;
		uint8_t interrupt_status : 1;
		uint8_t power_on_status : 1;
		const uint8_t reserved1 : 2;
	} __packed;

	uint8_t raw;
};

union meas_rate {
	struct {
		uint8_t rate : 3;
		const uint8_t reserved2 : 1;
		uint8_t resolution : 3;
		const uint8_t reserved1 : 1;
	} __packed;

	uint8_t raw;
};

union gain_range {
	struct {
		uint8_t gain : 3;
		const uint8_t reserved : 5;
	} __packed;

	uint8_t raw;
};

}
}