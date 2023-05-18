#include "sntp.hpp"

#include <esp_sntp.h>

namespace constant
{
const char ntp_server[] = "pool.ntp.org";
const uint32_t sntp_sync_interval_ms = 24 * 3600000; // 1 day
}

void sntp::init_time_sync()
{
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
	sntp_set_sync_interval(constant::sntp_sync_interval_ms);
	sntp_setservername(0, constant::ntp_server);
	sntp_init();
}

void sntp::disable_time_sync()
{
	sntp_stop();
}

bool sntp::is_sync_done()
{
	return sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
}