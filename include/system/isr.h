#ifndef SYSTEM_ISR_H
#define SYSTEM_ISR_H

#include <arch/armv7/registers.h>

enum exception {
	SUPERVISOR_CALL       = 0,
	PREFETCH_ABORT        = 1,
	DATA_ABORT            = 2,
	UNDEFINED_INSTRUCTION = 3,
};

void print_registers(struct registers* regs, char* exc_str,
                     char* exc_system_info_str, char* exc_extra_info_str);
void trigger_exception(enum exception exc);

#endif /* SYSTEM_ISR_H */
