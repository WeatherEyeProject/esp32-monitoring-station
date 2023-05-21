#include "data_collector.hpp"

#include <log.h>
#include <time.h>

#include "json_converter.hpp"

namespace constant
{
const uint32_t max_data_cout = 500;
}

void data_collector::add_new_data(sensor_data_list data)
{
	if (collected_data.size() == constant::max_data_cout) {
		logw(__func__, "Maximal data count reached, popping oldest result!");
		collected_data.pop_front();
	}

	data_packet packet;
	packet.data = data;
	packet.time = time(NULL);

	collected_data.push_back(packet);
}

bool data_collector::is_empty()
{
	return collected_data.size() == 0 && tmp_packet.size() == 0;
}

std::vector<char> data_collector::get_json_data_packet(const size_t max_count)
{
	if (tmp_packet.empty()) {
		auto packet_size = std::min(collected_data.size(), max_count);

		auto last = collected_data.end();
		auto first = std::prev(last, packet_size);

		tmp_packet.splice(tmp_packet.begin(), collected_data, first, last);
	}

	return json_converter::convert_data_to_json(tmp_packet);
}

void data_collector::trash_last_packet()
{
	tmp_packet = collected_data_list();
}