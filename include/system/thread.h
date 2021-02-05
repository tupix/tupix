#ifndef SYSTEM_THREAD_H
#define SYSTEM_THREAD_H

#include <config.h>

#include <arch/armv7/registers.h>

#include <driver/mmu.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>

enum thread_state { READY, WAITING, DONE };

struct tcb {
	size_t id, index;
	struct general_registers regs;
	uint32 cpsr;
	size_t waiting_duration;
	bool initialized;
	enum thread_state state;
	struct l2_entry l2_table;
};

void* get_stack_pointer(const size_t index);
void* get_max_stack_pointer(const size_t index);
struct tcb init_thread(void (*func)(void*));
void thread_create(void (*func)(void*), const void* args, size_t args_size);

#endif /* SYSTEM_THREAD_H */
