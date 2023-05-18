#include "wifi_connector.hpp"

#include <unordered_map>
#include <iostream>
#include <queue>
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include <storage/esp_nvs.hpp>

namespace constant
{
const char station_hostname[] = "WeatherEyeStation";
const uint8_t scan_retries = 5;
const std::unordered_map<std::string, std::string> known_networks = {
	#include "known_wifi_networks.txt"
};

}

bool wifi_connector::wifi_connected = false;
size_t wifi_connector::wifi_con_retries = 0;
std::mutex wifi_connector::wifi_op;
esp_netif_t* wifi_connector::sta_netif;
esp_event_handler_instance_t wifi_connector::instance_any_id;
esp_event_handler_instance_t wifi_connector::instance_got_ip;

void wifi_connector::event_handler(void* arg, esp_event_base_t event_base,
								   int32_t event_id, void* event_data)
{
	std::lock_guard<std::mutex> lock(wifi_op);

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		connect_to_known_ap();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		std::cout << "WiFi AP disconnected" << std::endl;
		wifi_connected = false;
		if (wifi_con_retries < constant::scan_retries) {
			esp_wifi_connect();
			wifi_con_retries++;
		}
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		std::cout << "WiFi got IP" << std::endl;
		wifi_con_retries = 0;
		wifi_connected = true;
	}
}

bool wifi_connector::init_wifi()
{
	if (!esp_nvs::init_nvs()) {
		return false;
	}

	auto ret = esp_netif_init();
	if (ret != ESP_OK) {
		return false;
	}

	ret = esp_event_loop_create_default();
	if (ret != ESP_OK) {
		return false;
	}

	sta_netif = esp_netif_create_default_wifi_sta();
	if (!sta_netif) {
		return false;
	}

	const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ret = esp_wifi_init(&cfg);
	if (ret != ESP_OK) {
		return false;
	}

	ret = esp_wifi_set_mode(WIFI_MODE_STA);
	if (ret != ESP_OK) {
		return false;
	}

	esp_event_handler_instance_register(WIFI_EVENT,
										ESP_EVENT_ANY_ID,
										event_handler,
										nullptr,
										&instance_any_id);
	esp_event_handler_instance_register(IP_EVENT,
										IP_EVENT_STA_GOT_IP,
										event_handler,
										nullptr,
										&instance_got_ip);

	ret = esp_netif_set_hostname(sta_netif, constant::station_hostname);
	if (ret != ESP_OK) {
		return false;
	}

	ret = esp_wifi_start();
	if (ret != ESP_OK) {
		return false;
	}

	return true;
}

bool wifi_connector::is_connected()
{
	std::lock_guard<std::mutex> lock(wifi_op);

	return wifi_connected;
}

bool wifi_connector::try_connect()
{
	std::lock_guard<std::mutex> lock(wifi_op);

	if (wifi_connected) {
		return true;
	}

	return connect_to_known_ap();
}

auto wifi_connector::do_ap_scan() -> std::vector<wifi_station>
{
	auto ret = esp_wifi_scan_start(NULL, true);
	if (ret != ESP_OK) {
		return {};
	}

	uint16_t found_aps;
	esp_wifi_scan_get_ap_num(&found_aps);
	if (found_aps == 0) {
		return {};
	}

	auto scan_result = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * found_aps);
	ret = esp_wifi_scan_get_ap_records(&found_aps, scan_result);
	if (ret != ESP_OK) {
		return {};
	}

	std::vector<wifi_station> station_list(found_aps);
	for (int i = 0; i < found_aps; i++) {
		station_list[i].ssid = std::string((char*)scan_result[i].ssid);
		station_list[i].rssi = scan_result[i].rssi;
	}

	free(scan_result);

	return station_list;
}

auto wifi_connector::choose_best_ap(const std::vector<wifi_station>& aps) -> const wifi_station*
{
	auto rssi_comp = [](const wifi_station* a, const wifi_station* b) {
		return a->rssi < b->rssi;
	};
	std::priority_queue<const wifi_station*, std::vector<const wifi_station*>, decltype(rssi_comp)> avail_stations(rssi_comp);

	for (auto& ap : aps) {
		if (constant::known_networks.contains(ap.ssid)) {
			avail_stations.push(&ap);
		}
	}
	if (avail_stations.empty()) {
		return nullptr;
	}

	return avail_stations.top();
}

bool wifi_connector::connect_to_known_ap()
{
	std::cout << "WiFi start scan" << std::endl;

	auto stations = do_ap_scan();
	if (stations.empty()) {
		return false;
	}

	auto best_station = choose_best_ap(stations);
	if (!best_station) {
		return false;
	}

	std::cout << "WiFi connecting to " << best_station->ssid << std::endl;

	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config));
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

	strcpy((char*)wifi_config.sta.ssid, best_station->ssid.c_str());
	strcpy((char*)wifi_config.sta.password, constant::known_networks.at(best_station->ssid).c_str());

	auto ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
	if (ret != ESP_OK) {
		return false;
	}

	ret = esp_wifi_connect();
	if (ret != ESP_OK) {
		return false;
	}

	return true;
}