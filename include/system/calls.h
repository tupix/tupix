#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include <arch/armv7/registers.h>
#include <data/types.h>

enum syscall_id {
	KILL_ME = 1,
	GET_CHAR = 2,
	PUT_CHAR = 3,
	WAIT = 4,
	CREATE_THREAD = 5,
};

uint16 get_syscall_id(uint32 lr);
void exec_syscall(uint16 id, struct registers* regs);

#endif /* SYSTEM_CALLS_H */
