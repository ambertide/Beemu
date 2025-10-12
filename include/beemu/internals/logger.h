#ifndef BEEMU_INTERNALS_LOGGER_H
#define BEEMU_INTERNALS_LOGGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	typedef enum BeemuLogLevel
	{
		BEEMU_LOG_ERR,
		BEEMU_LOG_WARN,
		BEEMU_LOG_INFO
	} BeemuLogLevel;

	/**
	 * @brief Log a message
	 *
	 * @param level Level of the message being logged.
	 * @param fmt Format of the message
	 * @param ... Message args
	 */
	void beemu_log(BeemuLogLevel level, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif // BEEMU_INTERNALS_LOGGER_H
