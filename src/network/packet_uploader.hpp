#pragma once

#include <vector>

class packet_uploader {
public:
	static bool upload_data(std::vector<char> data);
};