#pragma once

#include <vector>
#include <optional>

#include <driver/uart.h>

#include <sensor/sensor.hpp>

#include "pms7003_defs.hpp"

class pms7003 : public sensor
{
public:
	pms7003();
	~pms7003();

	bool init_sensor() override;
	bool wakeup() override;
	bool suspend() override;
	std::string get_name() const override;
	sensor_data_list read_data() override;

private:
	QueueHandle_t uart_queue;
	bool sensor_ready;
	bool sensor_suspended;

	std::optional<pms7003_internal::data_packet> get_sensor_data() const;
	bool send_host_command(const uint8_t command, const uint16_t data = 0x0000) const;
	void fix_uint16_endianess(uint8_t* data, const size_t size) const;
	bool check_data_packet(const pms7003_internal::data_packet& packet) const;
	bool verify_data_packet_checksum(const pms7003_internal::data_packet& packet) const;
	void wait_for_response_after_cmd();
	bool sensor_set_passive_mode();
};