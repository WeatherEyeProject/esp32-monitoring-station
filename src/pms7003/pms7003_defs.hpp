#pragma once

#include <string>
#include <driver/uart.h>

namespace constant
{
const std::string sensor_name("PMS7003");

const uint16_t start_sequence = 0x424d;

const uint8_t cmd_read_in_passive_mode = 0xe2;
const uint8_t cmd_change_mode = 0xe1;
const uint8_t cmd_sleep_set = 0xe4;

const uint16_t data_passive_mode = 0x0000;
const uint16_t data_active_mode = 0x0001;

const uint16_t data_sleep_set = 0x0000;
const uint16_t data_wakeup_set = 0x0001;

const uint8_t pms_rx_gpio = 17;
const uint8_t pms_tx_gpio = 16;
const uint8_t pms_set_gpio = 5;

const uart_config_t uart_config = {
	.baud_rate = 9600,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
	.rx_flow_ctrl_thresh = 122,
};

}

union data_packet {
	struct {
		uint16_t start_sequence;
		uint16_t frame_length;

		// CF=1 data is used for factory env
		uint16_t cf_pm_1_0;
		uint16_t cf_pm_2_5;
		uint16_t cf_pm_10;

		// Atmospheric env values
		uint16_t env_pm_1_0;
		uint16_t env_pm_2_5;
		uint16_t env_pm_10; //probably pm 10

		// Number of particles beyond certain uM
		// in 0.1 L of air
		uint16_t part_bey_0_3;
		uint16_t part_bey_0_5;
		uint16_t part_bey_1_0;
		uint16_t part_bey_2_5;
		uint16_t part_bey_5_0;
		uint16_t part_bey_10_0;

		uint16_t reserved;

		// Probably all previous values are summed up
		uint16_t check_code;
	} __packed;

	uint8_t raw[32];
};

union host_command {
	struct {
		uint16_t start_sequence;
		uint8_t command;
		uint16_t data;
		uint16_t verify_byte;
	} __packed;

	uint8_t raw[7];
};