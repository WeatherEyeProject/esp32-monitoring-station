#pragma once

#include <vector>
#include <string>

class packet_signer {
public:
	static std::vector<char> generate_data_signature(const std::vector<char>& data);

private:
	static std::vector<char> calculate_hmac(const std::vector<char>& data);
	static std::vector<char> encode_in_base64(const std::vector<char>& data);
};