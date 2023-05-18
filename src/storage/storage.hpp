#pragma once

class storage {
public:
	static bool init_nvs();

private:
	static bool nvs_initialized;
};