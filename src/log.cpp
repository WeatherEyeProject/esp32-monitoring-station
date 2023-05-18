#include "log.h"

#include <stdio.h>
#include <stdarg.h>

static const char level_letter[] = "\033[0;37mD\033[0;32mI\033[0;33mW\033[0;31mE";

static int current_log_level = 0;

void log_internal(int level, const char* fun, const char* msg, ...)
{
	if (level < current_log_level)
		return;

	printf("%.8s ", level_letter + (level * 8));
	printf("%s(): ", fun);

	va_list args;
	va_start(args, msg);

	vprintf(msg, args);

	va_end(args);

	printf("\033[m\n");
}