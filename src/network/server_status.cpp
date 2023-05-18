#include "server_status.hpp"

#include <iostream>

#include <cert/isrg_root_x1.h>

namespace constant
{
const char status_url[] = "https://weathereye.pl/api/status";
const char ok_response[] = "OK";
}

esp_err_t server_status::http_event_handler(esp_http_client_event_t* evt)
{
	if (evt->event_id != HTTP_EVENT_ON_DATA) {
		return ESP_OK;
	}

	if (evt->data_len <= 0) {
		return ESP_OK;
	}

	user_buf* buf = (user_buf*)evt->user_data;
	if (buf->buf_at == buf_size) {
		std::cout << "Buf full" << std::endl;
		return ESP_OK;
	}

	auto data_len = std::min(evt->data_len, buf_size - buf->buf_at);
	memcpy(buf->buf + buf->buf_at, evt->data, data_len);
	buf->buf_at += data_len;

	return ESP_OK;
}

bool server_status::check_server_status()
{
	bool ret = true;
	int status_code;

	user_buf buf;
	buf.buf_at = 0;

	esp_http_client_config_t config;
	memset(&config, 0, sizeof(config));
	config.url = constant::status_url;
	config.event_handler = http_event_handler;
	config.cert_pem = isrg_root_x1_cert;
	config.user_data = &buf;

	auto client = esp_http_client_init(&config);
	auto err = esp_http_client_perform(client);
	if (err != ESP_OK) {
		std::cout << "HTTPS error" << std::endl;
		ret = false;
		goto out;
	}

	status_code = esp_http_client_get_status_code(client);
	if (status_code != HttpStatus_Ok) {
		std::cout << "HTTPS status code: " << std::to_string(status_code) << std::endl;
		ret = false;
		goto out;
	}

	if (buf.buf_at != 2 || strncmp(buf.buf, constant::ok_response, buf.buf_at)) {
		ret = false;
	}

out:
	esp_http_client_cleanup(client);

	return ret;
}
