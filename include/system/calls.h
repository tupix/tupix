#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include <arch/armv7/registers.h>
#include <data/types.h>

#define _EXIT 1
#define _GET_CHAR 2
#define _PUT_CHAR 3
#define _WAIT 4
#define _FORK 5
#define _CREATE_THREAD 6

enum syscall_id {
	EXIT          = _EXIT,
	GET_CHAR      = _GET_CHAR,
	PUT_CHAR      = _PUT_CHAR,
	WAIT          = _WAIT,
	FORK          = _FORK,
	CREATE_THREAD = _CREATE_THREAD,
};

uint16 get_syscall_id(uint32 lr);
void exec_syscall(uint16 id, struct registers* regs);

#endif /* SYSTEM_CALLS_H */
