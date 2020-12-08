#ifndef STD_LOG_H
#define STD_LOG_H

enum log_lvl {
	ERROR,
	WARNING,
	LOG,
};

// [Reference](https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c)
#define log(log_lvl, fmt, ...)                                                 \
	do {                                                                       \
		if (DEBUG_ENABLED) {                                                   \
			kprintf("%s:%d:%s() ", __FILE__, __LINE__, __func__);              \
			switch (log_lvl) {                                                 \
			case ERROR:                                                        \
				kprintf("[ERROR]: ");                                          \
				break;                                                         \
			case WARNING:                                                      \
				kprintf("[WARNING]: ");                                        \
				break;                                                         \
			case LOG:                                                          \
			default:                                                           \
				kprintf("[LOG]: ");                                            \
			}                                                                  \
			kprintf(fmt, __VA_ARGS__);                                         \
			kprintf("\n");                                                     \
		}                                                                      \
	} while (0)

#endif
