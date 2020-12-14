#ifndef SYSTEM_THREAD_H
#define SYSTEM_THREAD_H

#include <config.h>

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>

struct tcb {
	size_t id;
	void (*callback)(void*);
	struct general_registers regs;
	// TODO: Is there more we need?
};

struct tcb create_thread();
#if 0
#define THREAD_STACK_BASE // TODO
#define THREAD_STACK_SIZE 4KB

struct tcb thread_create(void (*func)(void*), const void* args,
						 uint32 args_size);
#endif

void run(struct tcb* thread);

#endif /* SYSTEM_THREAD_H */
