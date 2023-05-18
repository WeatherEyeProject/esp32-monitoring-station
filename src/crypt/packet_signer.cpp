#include "packet_signer.hpp"

#include <mbedtls/md.h>
#include <mbedtls/base64.h>

#include <iostream>

namespace constant
{
const uint32_t hmac_key_size = 32; // 256 bits
const uint32_t base64_encoded_hmac_size = 44 + 1; // 1 for null termination
const uint8_t hmac_key[hmac_key_size] = {
	#include "secret_hmac_key.txt"
};
const auto ctx_digest = MBEDTLS_MD_SHA256;
}

std::vector<char> packet_signer::generate_data_signature(const std::vector<char>& data)
{
	auto hmac = calculate_hmac(data);
	if (hmac.size() == 0) {
		return hmac;
	}

	return encode_in_base64(hmac);
}

std::vector<char> packet_signer::calculate_hmac(const std::vector<char>& data)
{
	mbedtls_md_context_t ctx;
	int ret;
	std::vector<char> hmac(constant::hmac_key_size);

	mbedtls_md_init(&ctx);
	ret = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(constant::ctx_digest), 1);
	if (ret != 0) {
		hmac = std::vector<char>();
		goto out;
	}
	
	mbedtls_md_hmac_starts(&ctx, constant::hmac_key, constant::hmac_key_size);
	mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.data(), data.size());
	mbedtls_md_hmac_finish(&ctx, (unsigned char*)hmac.data());

out:
	mbedtls_md_free(&ctx);

	return hmac;
}

std::vector<char> packet_signer::encode_in_base64(const std::vector<char>& data)
{
	size_t written;
	auto base64_out = std::vector<char>(constant::base64_encoded_hmac_size);

	auto ret = mbedtls_base64_encode((unsigned char*)base64_out.data(), constant::base64_encoded_hmac_size,
									 &written, (const unsigned char*)data.data(), data.size());
	if (ret != 0) {
		return {};
	}

	return base64_out;
}