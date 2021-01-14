#ifndef SYSTEM_THREAD_H
#define SYSTEM_THREAD_H

#include <config.h>

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>

// NOTE: Below user stack
#define THREAD_STACK_BASE 0x7FFE800
#define THREAD_STACK_SIZE 0x400 // 4KB

enum thread_state { READY, WAITING, DONE };

struct tcb {
	size_t id, index;
	void (*callback)(void*);
	struct general_registers regs;
	uint32 cpsr;
	bool initialized;
	enum thread_state state;
};

void* get_stack_pointer(const size_t index);
void* get_max_stack_pointer(const size_t index);
void thread_create(void (*func)(void*), const void* args, size_t args_size);

#endif /* SYSTEM_THREAD_H */
