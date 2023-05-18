#include "storage.hpp"

#include "nvs_flash.h"

bool storage::nvs_initialized = false;

bool storage::init_nvs()
{
	if (nvs_initialized)
		return true;
	
	auto ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ret = nvs_flash_erase();
		if (ret != ESP_OK) {
			return false;
		}

		ret = nvs_flash_init();
		if (ret != ESP_OK) {
			return false;
		}
	}
	else if (ret != ESP_OK) {
		return false;
	}

	nvs_initialized = true;

	return true;
}