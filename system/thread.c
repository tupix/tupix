#include <system/scheduler.h>
#include <system/thread.h>

#include <std/log.h>
#include <std/mem.h>

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
	size_t id = *(size_t*)stack;
	for (uint32 i = 0;; ++i) {
		log(LOG, "thread: %i: %i", id, i);
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER_SCHEDULER; ++i) {}
	}
}
