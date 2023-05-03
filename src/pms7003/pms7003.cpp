#include "pms7003.hpp"

#include <iostream>
#include <endian.h>

#include "pms7003_defs.hpp"
using namespace pms7003_internal;

namespace pms7003_internal::constant
{
const uart_port_t uart_num = UART_NUM_1;
const int uart_rx_buffer_size = (128 * 2);
}

pms7003::pms7003()
	: sensor_ready(false),
	sensor_suspended(false)
{ }

pms7003::~pms7003()
{
	uart_driver_delete(constant::uart_num);
}

std::string pms7003::get_name() const
{
	return constant::sensor_name;
}

bool pms7003::init_sensor()
{
	auto ret = uart_param_config(constant::uart_num, &constant::uart_config);
	if (ret != ESP_OK) {
		std::cout << "UART init error" << std::endl;
		return false;
	}

	ret = uart_set_pin(constant::uart_num, constant::pms_rx_gpio, constant::pms_tx_gpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	if (ret != ESP_OK) {
		std::cout << "UART set pin error" << std::endl;
		return false;
	}

	ret = uart_driver_install(constant::uart_num, constant::uart_rx_buffer_size,
							  0, 10, &uart_queue, 0);
	if (ret != ESP_OK) {
		std::cout << "UART driver install error" << std::endl;
		return false;
	}

	sensor_ready = true;

	suspend();

	return true;
}

sensor_data_list pms7003::read_data()
{
	if (!sensor_ready) {
		return {};
	}

	if (sensor_suspended) {
		return {};
	}

	const auto raw_data_opt = get_sensor_data();
	if (!raw_data_opt) {
		std::cout << "No sensor data" << std::endl;
		return {};
	}
	const auto& raw_data = *raw_data_opt;

	sensor_data_list data;
	data.push_back(sensor_data(environmental_data::type::AIR_PM_1, raw_data.env_pm_1_0));
	data.push_back(sensor_data(environmental_data::type::AIR_PM_2_5, raw_data.env_pm_2_5));
	data.push_back(sensor_data(environmental_data::type::AIR_PM_10, raw_data.env_pm_10));
	return data;
}

bool pms7003::send_host_command(const uint8_t command, const uint16_t data) const
{
	host_command cmd = { .raw = 0 };

	cmd.start_sequence = htobe16(constant::start_sequence);
	cmd.command = command;
	cmd.data = htobe16(data);

	uint16_t checksum = 0;
	const size_t cmd_size_without_checksum = sizeof(host_command) - sizeof(host_command::verify_byte);
	for (size_t i = 0; i < cmd_size_without_checksum; i++)
		checksum += cmd.raw[i];
	cmd.verify_byte = htobe16(checksum);

	auto ret = uart_write_bytes(constant::uart_num, &cmd.raw, sizeof(host_command));
	if (ret != sizeof(host_command)) {
		std::cout << "UART bytes write error" << std::endl;
		return false;
	}

	return true;
}

bool pms7003::suspend()
{
	if (!sensor_ready) {
		return false;
	}

	if (sensor_suspended) {
		return true;
	}

	std::cout << "PMS7003 suspending..." << std::endl;
	auto ret = send_host_command(constant::cmd_sleep_set, constant::data_sleep_set);
	if (ret)
		sensor_suspended = true;

	wait_for_response_after_cmd();
	std::cout << "Ok" << std::endl;

	return ret;
}

bool pms7003::wakeup()
{
	if (!sensor_ready) {
		return false;
	}

	if (!sensor_suspended) {
		return true;
	}

	std::cout << "PMS7003 waking up..." << std::endl;
	auto ret = send_host_command(constant::cmd_sleep_set, constant::data_wakeup_set);
	if (!ret)
		return ret;

	sensor_suspended = false;
	wait_for_response_after_cmd();

	ret = sensor_set_passive_mode();
	if (!ret)
		return ret;

	std::cout << "Ok" << std::endl;

	return ret;
}

bool pms7003::sensor_set_passive_mode()
{
	auto ret = send_host_command(constant::cmd_change_mode, constant::data_passive_mode);
	if (ret != true) {
		std::cout << "Passive mode set error" << std::endl;
		return false;
	}
	wait_for_response_after_cmd();

	return true;
}

std::optional<data_packet> pms7003::get_sensor_data() const
{
	send_host_command(constant::cmd_read_in_passive_mode);

	data_packet data = { .raw = 0 };
	auto length = uart_read_bytes(constant::uart_num, &data, sizeof(data_packet), 100);
	if (length != sizeof(data_packet)) {
		std::cout << "Unexpected packet length " << std::to_string(length) << std::endl;
		std::cout << std::hex << *(uint64_t*)data.raw << std::endl;
		return std::nullopt;
	}

	fix_uint16_endianess(data.raw, sizeof(data_packet));

	if (!check_data_packet(data)) {
		return std::nullopt;
	}

	return data;
}

void pms7003::fix_uint16_endianess(uint8_t* data, const size_t size) const
{
	for (size_t i = 0; i < size; i += 2) {
		std::swap(data[i], data[i + 1]);
	}
}

bool pms7003::check_data_packet(const pms7003_internal::data_packet& packet) const
{
	if (!verify_data_packet_checksum(packet)) {
		std::cout << "Invalid checksum" << std::endl;
		return false;
	}

	if (packet.start_sequence != constant::start_sequence) {
		std::cout << "Start sequence invalid, got: " << std::hex << packet.start_sequence << std::dec << std::endl;
		return false;
	}

	if (packet.frame_length != constant::data_packet_frame_length) {
		std::cout << "Frame length invalid, got: " << std::to_string(packet.frame_length) << std::endl;
		return false;
	}

	return true;
}

bool pms7003::verify_data_packet_checksum(const pms7003_internal::data_packet& packet) const
{
	const size_t packet_size_without_checksum = sizeof(data_packet) - sizeof(data_packet::check_code);
	uint16_t checksum = 0;
	for (size_t i = 0; i < packet_size_without_checksum; i++) {
		checksum += packet.raw[i];
	}

	return checksum == packet.check_code;
}

void pms7003::wait_for_response_after_cmd()
{
	uint8_t data[8];
	int ret = uart_read_bytes(constant::uart_num, &data, sizeof(data), pdMS_TO_TICKS(5000));
	if (ret < 0) {
		std::cout << "Waiting for PM7003 response timeout" << std::endl;
	}
	uart_flush(constant::uart_num);
}