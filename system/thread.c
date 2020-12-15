#include <system/thread.h>

#include <data/types.h>
#include <system/scheduler.h>

#include <std/log.h>
#include <std/mem.h>

void
thread_create(void (*func)(void*), const void* args, size_t args_size)
{
	struct tcb thread  = { 0 };
	thread.callback	   = func;
	thread.regs.lr	   = (uint32)func;
	thread.initialized = false;
	// TODO: What else is there to be done?

	struct tcb* scheduled_thread = schedule_thread(&thread);
	if (!scheduled_thread)
		return; // Thread was not added to queue

	// All thread stacks are positioned on top of each other with stack for id 0
	// at the very top.
	void* thread_sp = (void*)(THREAD_STACK_BASE);
	thread_sp -= scheduled_thread->id * THREAD_STACK_SIZE;

	// Since the stack grows to the 'bottom', copy below it
	thread_sp -= args_size;
	memcpy(thread_sp, args, args_size);

	// Copy thread id onto stack for now too, as dummy_run uses it.
	thread_sp -= sizeof(scheduled_thread->id);
	memcpy(thread_sp, &scheduled_thread->id, sizeof(scheduled_thread->id));

	// Update stack pointer
	scheduled_thread->regs.sp = (uint32)thread_sp;

	// Pass stack-pointer as argument
	scheduled_thread->regs.r0	  = scheduled_thread->regs.sp;
	scheduled_thread->initialized = true;
}

#define PRINT_N 10
#define BUSY_WAIT_COUNTER_SCHEDULER 50000000
void
dummy_run(void* stack)
{
	size_t id = *(size_t*)stack;
	for (uint32 i = 0;; ++i) {
		log(LOG, "thread: %i: %i", id, i);
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER_SCHEDULER; ++i) {}
	}
}
