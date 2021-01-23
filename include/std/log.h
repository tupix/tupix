#ifndef STD_LOG_H
#define STD_LOG_H

#include <config.h>

#include <std/io.h>

enum klog_lvl {
	ASSERT  = 0,
	ERROR   = 1,
	WARNING = 2,
	LOG     = 3,
	DEBUG   = 4,
};

// [Reference](https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c)
#define klog(klog_lvl, fmt, ...)                                               \
	do {                                                                       \
		if (LOGGING_ENABLED && klog_lvl <= LOGGING_LEVEL) {                    \
			switch (klog_lvl) {                                                \
			case ASSERT:                                                       \
				kprintf("\e[0;31m[ASSERT] ");                                  \
				break;                                                         \
			case ERROR:                                                        \
				kprintf("\e[0;31m[ERROR] ");                                   \
				break;                                                         \
			case WARNING:                                                      \
				kprintf("\e[0;33m[WARNING] ");                                 \
				break;                                                         \
			case LOG:                                                          \
			default:                                                           \
				kprintf("[LOG] ");                                             \
				break;                                                         \
			case DEBUG:                                                        \
				kprintf("\e[0;36m[DEBUG] ");                                   \
				break;                                                         \
			}                                                                  \
			kprintf("%s:%d:%s(): ", __FILE__, __LINE__, __func__);             \
			kprintf(fmt, ##__VA_ARGS__);                                       \
			kprintf("\e[0m\n");                                                \
		}                                                                      \
	} while (0)

#endif
