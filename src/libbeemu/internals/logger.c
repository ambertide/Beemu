#include <libbeemu/internals/logger.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief Print log level.
 *
 * @param level
 */
void print_prelude(BeemuLogLevel level)
{
	switch (level)
	{
	case BEEMU_LOG_INFO:
		printf("INFO: ");
		break;
	case BEEMU_LOG_ERR:
		printf("ERROR: ");
		break;
	case BEEMU_LOG_WARN:
		printf("WARN: ");
		break;
	}
}

void beemu_log(BeemuLogLevel level, const char *fmt, ...)
{
#ifndef DDEBUG
	print_prelude(level);
	va_list fmt_args;
	va_start(fmt_args, fmt);
	vprintf(fmt, fmt_args);
	va_end(fmt_args);
	printf("\n");
#endif
}