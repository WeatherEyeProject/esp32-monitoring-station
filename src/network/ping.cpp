#if 0
#include "ping.hpp"

#include <cstring>

#include <netdb.h>

ping::ping()
{
	ping_config = ESP_PING_DEFAULT_CONFIG();

	ping_callbacks.on_ping_success = nullptr;
	ping_callbacks.on_ping_timeout = nullptr;
	ping_callbacks.on_ping_end = on_ping_end;
	ping_callbacks.cb_args = this;
}

bool ping::set_address(const std::string& url)
{
	ip_addr_t host_address;
	addrinfo hint;
	addrinfo* res = NULL;

	memset(&hint, 0, sizeof(hint));
	memset(&host_address, 0, sizeof(host_address));

	auto ret = getaddrinfo(url.c_str(), NULL, &hint, &res);
	if (ret != 0) {
		return false;
	}
	
	in_addr addr4 = ((struct sockaddr_in*)(res->ai_addr))->sin_addr;
	inet_addr_to_ip4addr(ip_2_ip4(&host_address), &addr4);
	freeaddrinfo(res);

	ping_config.target_addr = host_address;

	return true;
}

bool ping::do_ping(uint32_t count)
{
	if (count == 0)
		return false;

	ping_config.count = count;

	esp_ping_handle_t ping_session;
	auto ret = esp_ping_new_session(&ping_config, &ping_callbacks, &ping_session);
	if (ret != ESP_OK) {
		return false;
	}

	ping_promise = std::promise<bool>();
	auto ping_result = ping_promise.get_future();

	ret = esp_ping_start(ping_session);
	if (ret != ESP_OK) {
		return false;
	}

	ping_result.wait();

	esp_ping_stop(ping_session);
	esp_ping_delete_session(ping_session);

	return ping_result.get();
}

void ping::on_ping_end(esp_ping_handle_t hdl, void* args)
{
	ping* content = (ping*)args;
	uint32_t received;

	esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));

	content->ping_promise.set_value(content->ping_config.count == received);
}
#endif