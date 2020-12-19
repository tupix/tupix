#ifndef SYSTEM_ISR_H
#define SYSTEM_ISR_H

enum exception {
	SUPERVISOR_CALL       = 0,
	PREFETCH_ABORT        = 1,
	DATA_ABORT            = 2,
	UNDEFINED_INSTRUCTION = 3,
};

void trigger_exception(enum exception exc);

#endif /* SYSTEM_ISR_H */
