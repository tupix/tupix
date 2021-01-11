#include <config.h>

#include <system/thread.h>

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/entry.h>
#include <system/ivt.h>
#include <system/scheduler.h>

#include <std/log.h>
#include <std/mem.h>

void*
get_stack_pointer(const size_t index)
{
	// All thread stacks are positioned on top of each other with stack for id 0
	// at the very top.

	// Respect max number of threads
	if (index >= N_THREADS)
		return NULL;
	// Do not try to create a pointer of negative memory address
	if (THREAD_STACK_BASE - index * THREAD_STACK_SIZE < 0)
		return NULL;
	void* stack = (void*)(THREAD_STACK_BASE);
	stack -= index * THREAD_STACK_SIZE;
	return stack;
}

void*
get_max_stack_pointer(const size_t index)
{
	return get_stack_pointer(index + 1) + 1;
}

void
exit_thread()
{
	// TODO: Disable timer?
	asm("svc #1");
}

void
thread_create(void (*func)(void*), const void* args, size_t args_size)
{
	struct tcb* scheduled_thread;
	{
		struct tcb thread  = { 0 };
		thread.callback    = func;
		thread.regs.pc     = (uint32)func;
		thread.regs.lr     = (uint32)&exit_thread;
		thread.cpsr        = PROCESSOR_MODE_USR;
		thread.initialized = false;

		scheduled_thread = schedule_thread(&thread);
	}
	if (!scheduled_thread)
		return; // Thread was not added to queue

	void* thread_sp = get_stack_pointer(scheduled_thread->index);

	// Since the stack grows to the 'bottom', copy below it
	thread_sp -= args_size;
	memcpy(thread_sp, args, args_size);

	// Update stack pointer
	scheduled_thread->regs.sp = (uint32)thread_sp;

	// Pass stack-pointer as argument
	scheduled_thread->regs.r0     = scheduled_thread->regs.sp;
	scheduled_thread->initialized = true;
}
