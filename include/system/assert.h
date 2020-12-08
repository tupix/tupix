#ifndef SYS_ASSERT_H
#define SYS_ASSERT_H

#include <system/kernel.h>

#include <std/log.h>

#define ASSERT(condition)                                                      \
	do {                                                                       \
		if (!condition) {                                                      \
			log(ASSERT, "");                                                   \
			PANIC;                                                             \
		}                                                                      \
	} while (0)

#define ASSERTM(condition, msg)                                                \
	do {                                                                       \
		if (!condition) {                                                      \
			log(ASSERT, "%s\n", msg);                                          \
			PANIC;                                                             \
		}                                                                      \
	} while (0)

#endif
