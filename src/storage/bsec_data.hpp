#pragma once

#include <vector>

#include <nvs_flash.h>

class bsec_data {
public:
	~bsec_data();

	bool init_partition();

	uint16_t get_state_version();
	std::vector<uint8_t> get_learned_state();

	bool erase_partition();

	bool save_learned_state(const std::vector<uint8_t>& state);

private:
	nvs_handle_t handle;
	uint16_t learning_revision;
};