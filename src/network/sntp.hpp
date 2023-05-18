#pragma once


class sntp {
public:
	static void init_time_sync();
	static void disable_time_sync();
	static bool is_sync_done();
};