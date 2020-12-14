#include <system/scheduler.h>
#include <system/thread.h>

#include <std/log.h>
#include <std/mem.h>

struct tcb
create_thread()
{
	struct tcb thread = { 0 };
	thread.callback	  = (void*)&run;
	return thread;
}

void
thread_create(void (*func)(void*), const void* args, size_t args_size)
{
	struct tcb thread = { 0 };
	thread.callback	  = func;
	thread.regs.pc	  = (uint32)func;
	// TODO: What else is there to be done?

	struct tcb* scheduled_thread = schedule_thread(&thread);
	if (scheduled_thread == &thread)
		return; // Thread was not added to queue

	// All thread stacks are positioned on top of each other with stack for id 0
	// at the very top.
	void* thread_stack = (void*)(THREAD_STACK_BASE);
	thread_stack -= scheduled_thread->id * THREAD_STACK_SIZE;
	// Since the stack grows to the 'bottom', copy below it
	memcpy(thread_stack - args_size, args, args_size);
	// Update stack pointer
	scheduled_thread->regs.sp = (uint32)(thread_stack - args_size);
}

#define PRINT_N 10
#define BUSY_WAIT_COUNTER_SCHEDULER 10000000
void
dummy_run(void* stack)
{
	for (uint32 i = 0;; ++i) {
		log(LOG, "dummy run");
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER_SCHEDULER; ++i) {}
	}
}

void
run(struct tcb* thread)
{
	for (uint32 i = 0;; ++i) {
		log(LOG, "thread %i: %i", thread->id, i);
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER_SCHEDULER; ++i) {}
	}
}
