#ifndef SYSTEM_THREAD_H
#define SYSTEM_THREAD_H

#include <arch/armv7/registers.h>

struct tcb {
	// TODO
	void (*callback)(void*);
	struct general_registers regs;
};

#endif /* SYSTEM_THREAD_H */
