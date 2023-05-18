#pragma once

extern void log_internal(int level, const char* fun, const char* msg, ...);

#define logd(...) log_internal(0, __VA_ARGS__)
#define logi(...) log_internal(1, __VA_ARGS__)
#define logw(...) log_internal(2, __VA_ARGS__)
#define loge(...) log_internal(3, __VA_ARGS__)