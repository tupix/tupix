#ifndef SYS_KERNEL_H
#define SYS_KERNEL_H

#define PANIC                                                                  \
	do {                                                                       \
		/* TODO(Aurel): Pull out 0xff into a header. */                        \
		asm volatile("udf #0xff");                                             \
	} while (0)

#endif
