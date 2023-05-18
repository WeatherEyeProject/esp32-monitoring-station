#if 0
#pragma once

#include <string>
#include <future>

#include <ping/ping_sock.h>

class ping {
public:
	ping();

	bool set_address(const std::string& url);
	bool do_ping(uint32_t count = 1);

private:
	esp_ping_config_t ping_config;
	esp_ping_callbacks_t ping_callbacks;
	std::promise<bool> ping_promise;

	static void on_ping_end(esp_ping_handle_t hdl, void* args);
};
#endif