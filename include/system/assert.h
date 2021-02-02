#ifndef SYS_ASSERT_H
#define SYS_ASSERT_H

#include <system/kernel.h>

#include <std/log.h>

#define ASSERT(condition)                                                      \
	do {                                                                       \
		if (!(condition)) {                                                    \
			klog(ASSERT, "");                                                  \
			PANIC;                                                             \
		}                                                                      \
	} while (0)

#define ASSERTM(condition, fmt, ...)                                           \
	do {                                                                       \
		if (!(condition)) {                                                    \
			klog(ASSERT, fmt, ##__VA_ARGS__);                                  \
			klog(ASSERT, "Panicing.");                                         \
			PANIC;                                                             \
		}                                                                      \
	} while (0)

#endif
