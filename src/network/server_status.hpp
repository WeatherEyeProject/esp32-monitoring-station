#pragma once

#include <esp_http_client.h>

class server_status {
public:
	static bool check_server_status();

private:
	static const uint8_t buf_size = 2;
	struct user_buf {
		char buf[buf_size];
		uint8_t buf_at;
	};

	static esp_err_t http_event_handler(esp_http_client_event_t* evt);
};