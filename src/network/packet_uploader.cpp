#include "packet_uploader.hpp"

#include <iostream>

#include <esp_http_client.h>

#include <cert/isrg_root_x1.h>
#include <crypt/packet_signer.hpp>

namespace constant
{
const char status_url[] = "https://weathereye.pl/api/SensorsDataGatherer";
}

bool packet_uploader::upload_data(std::vector<char> data)
{
	bool ret = true;
	esp_err_t err;
	int status_code;

	esp_http_client_config_t config;
	memset(&config, 0, sizeof(config));
	config.url = constant::status_url;
	config.cert_pem = isrg_root_x1_cert;
	config.method = HTTP_METHOD_POST;
	config.disable_auto_redirect = true;
	config.timeout_ms = 2000;

	auto client = esp_http_client_init(&config);

	esp_http_client_set_header(client, "Content-Type", "application/json");

	auto signature = packet_signer::generate_data_signature(data);
	if (signature.size() == 0) {
		ret = false;
		goto out;
	}
	err = esp_http_client_set_header(client, "Authorization", signature.data());
	if (err != ESP_OK) {
		ret = false;
		goto out;
	}

	err = esp_http_client_set_post_field(client, data.data(), data.size());
	if (err != ESP_OK) {
		ret = false;
		goto out;
	}

	err = esp_http_client_perform(client);
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

out:
	esp_http_client_cleanup(client);

	return ret;
}