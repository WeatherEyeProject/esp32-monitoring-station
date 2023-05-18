#pragma once

class esp_nvs {
public:
	static bool init_nvs();

private:
	static bool nvs_initialized;
};