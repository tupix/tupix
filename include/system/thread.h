#ifndef SYSTEM_THREAD_H
#define SYSTEM_THREAD_H

#include <config.h>

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>

// TODO: Do not hardcode (sp_usr - STACK_SIZE)
#define THREAD_STACK_BASE 0x7FFE800
#define THREAD_STACK_SIZE 0x400 // 4KB

struct tcb {
	size_t id;
	size_t index;
	void (*callback)(void*);
	struct general_registers regs;
	// TODO: Is there more we need?
	bool initialized;
};

void thread_create(void (*func)(void*), const void* args, size_t args_size);
void dummy_run(void* stack);

#endif /* SYSTEM_THREAD_H */
