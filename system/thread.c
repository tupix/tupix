#include "std/io.h"
#include <system/thread.h>

#include <data/types.h>
#include <system/entry.h>
#include <system/scheduler.h>

#include <std/log.h>
#include <std/mem.h>

// TODO: Own header and source file?
#define PRINT_N 10
void
user_thread(void* x)
{
	size_t id = *(size_t*)x;
	x += sizeof(id);
	uint32 c = *(uint32*)x;
	x += sizeof(c);
	for (size_t i = 0; i < PRINT_N; ++i) {
		log(DEBUG, "thread %i: %c", id, c);
		kprintf("%c", c);
		// We need a volatile counter so that the loop is not optimized out.
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER; ++i) {}
	}
	log(DEBUG, "thread %i: done", id);
	return;
}

void
thread_create(void (*func)(void*), const void* args, size_t args_size)
{
	struct tcb thread = { 0 };
	thread.callback   = func;
	thread.regs.pc    = (uint32)func;
	thread.regs.lr    = (uint32)&endless_loop;
	// thread.cpsr = spsr;
	thread.initialized = false;
	// TODO: What else is there to be done?

	struct tcb* scheduled_thread = schedule_thread(&thread);
	if (!scheduled_thread)
		return; // Thread was not added to queue

	// All thread stacks are positioned on top of each other with stack for id 0
	// at the very top.
	void* thread_sp = (void*)(THREAD_STACK_BASE);
	thread_sp -= scheduled_thread->index * THREAD_STACK_SIZE;

	// Since the stack grows to the 'bottom', copy below it
	thread_sp -= args_size;
	memcpy(thread_sp, args, args_size);

	// Copy thread id onto stack for now too, as dummy_run uses it.
	thread_sp -= sizeof(scheduled_thread->id);
	memcpy(thread_sp, &scheduled_thread->id, sizeof(scheduled_thread->id));

	// Update stack pointer
	scheduled_thread->regs.sp = (uint32)thread_sp;

	// Pass stack-pointer as argument
	scheduled_thread->regs.r0     = scheduled_thread->regs.sp;
	scheduled_thread->initialized = true;
}
