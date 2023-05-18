#pragma once

#include <mutex>
#include <vector>

#include "esp_wifi.h"

class wifi_connector {
public:
	static bool init_wifi();
	static bool is_connected();

	// Returns true when wifi is connected or connection attempt
	// will be made (check with is_connected()), false if no AP available
	static bool try_connect();

private:
	static std::mutex wifi_op;
	static esp_netif_t* sta_netif;
	static esp_event_handler_instance_t instance_any_id;
	static esp_event_handler_instance_t instance_got_ip;
	static bool wifi_connected;
	static size_t wifi_con_retries;

	struct wifi_station {
		std::string ssid;
		uint8_t rssi;
	};

	static std::vector<wifi_station> do_ap_scan();
	static const wifi_station* choose_best_ap(const std::vector<wifi_station>& aps);
	static bool connect_to_known_ap();
	static void event_handler(void* arg, esp_event_base_t event_base,
							  int32_t event_id, void* event_data);
};