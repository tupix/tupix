#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include <data/types.h>

uint16 get_syscall_id(uint32 lr);
void exec_syscall(uint16 id);

#endif /* SYSTEM_CALLS_H */
