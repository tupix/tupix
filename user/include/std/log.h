#ifndef STD_LOG_H
#define STD_LOG_H

#include <config.h>

#include <std/io.h>

enum log_lvl {
	ASSERT  = 0,
	ERROR   = 1,
	WARNING = 2,
	LOG     = 3,
	DEBUG   = 4,
};

// [Reference](https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c)
#define log(log_lvl, fmt, ...)                                                 \
	do {                                                                       \
		if (LOGGING_ENABLED && log_lvl <= LOGGING_LEVEL) {                     \
			switch (log_lvl) {                                                 \
			case ASSERT:                                                       \
				kprintf("[ASSERT] ");                                          \
				break;                                                         \
			case ERROR:                                                        \
				kprintf("[ERROR] ");                                           \
				break;                                                         \
			case WARNING:                                                      \
				kprintf("[WARNING] ");                                         \
				break;                                                         \
			case LOG:                                                          \
			default:                                                           \
				kprintf("[LOG] ");                                             \
				break;                                                         \
			case DEBUG:                                                        \
				kprintf("[DEBUG] ");                                           \
				break;                                                         \
			}                                                                  \
			kprintf("%s:%d:%s(): ", __FILE__, __LINE__, __func__);             \
			kprintf(fmt, ##__VA_ARGS__);                                       \
			kprintf("\n");                                                     \
		}                                                                      \
	} while (0)

#endif
