#include "bsec_data.hpp"

#include <log.h>

namespace constant
{
const char partition_name[] = "bsec_algo";
const char data_revision_key_name[] = "data_rev";
const char learned_state_key_name[] = "state";
const char namespace_name[] = "bsec";
const size_t learning_state_size = 155;
}

bsec_data::~bsec_data()
{
	nvs_flash_deinit_partition(constant::partition_name);
}

bool bsec_data::init_partition()
{
	auto ret = nvs_flash_init_partition(constant::partition_name);
	if (ret != ESP_OK) {
		loge(__func__, "bsec_algo init failed");
		if (!erase_partition()) {
			return false;
		}
	}

	ret = nvs_open_from_partition(constant::partition_name, constant::namespace_name, NVS_READWRITE, &handle);
	if (ret != ESP_OK) {
		loge(__func__, "bsec_algo open failed");
		return false;
	}

	ret = nvs_get_u16(handle, constant::data_revision_key_name, &learning_revision);
	if (ret == ESP_ERR_NVS_NOT_FOUND) {
		logi(__func__, "Data revision key not found, init with 0");
		ret = nvs_set_u16(handle, constant::data_revision_key_name, 0);
		ret |= nvs_commit(handle);
		if (ret != ESP_OK) {
			loge(__func__, "Setting data rev key failed");
			return false;
		}
		learning_revision = 0;
	}
	else if (ret != ESP_OK) {
		loge(__func__, "Data rev key read error");
		return false;
	}

	return true;
}

bool bsec_data::erase_partition()
{
	logi(__func__, "bsec_algo erase requested");
	auto ret = nvs_flash_erase_partition(constant::partition_name);
	if (ret != ESP_OK) {
		loge(__func__, "bsec_algo erase failed");
		return false;
	}

	ret = nvs_flash_init_partition(constant::partition_name);
	if (ret != ESP_OK) {
		loge(__func__, "bsec_algo init failed after erase");
		return false;
	}

	return true;
}

uint16_t bsec_data::get_state_version()
{
	return learning_revision;
}

std::vector<uint8_t> bsec_data::get_learned_state()
{
	std::vector<uint8_t> data(constant::learning_state_size);
	auto size = constant::learning_state_size;

	auto ret = nvs_get_blob(handle, constant::learned_state_key_name, data.data(), &size);

	if (size != constant::learning_state_size || ret != ESP_OK) {
		loge(__func__, "Read learned state failed, ret %d data %u", ret, size);
		return {};
	}

	return data;
}

bool bsec_data::save_learned_state(const std::vector<uint8_t>& state)
{
	auto ret = nvs_set_blob(handle, constant::learned_state_key_name, state.data(), state.size());
	if (ret != ESP_OK) {
		loge(__func__, "Save learned state failed, ret %d", ret);
		return false;
	}

	learning_revision++;
	ret = nvs_set_u16(handle, constant::data_revision_key_name, learning_revision);
	if (ret != ESP_OK) {
		loge(__func__, "Save data revision failed");
		return false;
	}

	ret = nvs_commit(handle);
	if (ret != ESP_OK) {
		loge(__func__, "Commit data failed");
		return false;
	}

	return true;
}