#include <config.h>

#include <system/thread.h>

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/entry.h>
#include <system/ivt.h>
#include <system/scheduler.h>

#include <std/log.h>
#include <std/mem.h>

extern void exit();

void*
get_stack_pointer(const size_t index)
{
	// All thread stacks are positioned on top of each other with stack for id 0
	// at the very top.

	// Respect max number of threads
	if (index >= N_THREADS)
		return NULL;
	// Do not try to create a pointer of negative memory address
	if (THREAD_STACK_BASE - (int32)index * THREAD_STACK_SIZE < 0)
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

struct tcb
init_thread(void (*func)(void*))
{
	struct tcb thread       = { 0 };
	thread.callback         = func;
	thread.regs.pc          = (uint32)func;
	thread.regs.lr          = (uint32)&exit;
	thread.cpsr             = PROCESSOR_MODE_USR;
	thread.waiting_duration = 0;
	thread.initialized      = false;

	return thread;
}

void
thread_create(void (*func)(void*), const void* args, size_t args_size)
{
	struct tcb* scheduled_thread = schedule_thread(init_thread(func));
	if (!scheduled_thread)
		return; // Thread was not added to queue

	void* thread_sp = get_stack_pointer(scheduled_thread->index);

	// Since the stack grows to the 'bottom', copy below it
	// TODO: What to do if we cannot access user memory?
	if (args && args_size) {
		thread_sp -= args_size;
		kmemcpy(thread_sp, args, args_size);
	}

	// Update stack pointer
	scheduled_thread->regs.sp = (uint32)thread_sp;

	// Pass stack-pointer as argument
	if (args && args_size)
		scheduled_thread->regs.r0 = scheduled_thread->regs.sp;
	else
		scheduled_thread->regs.r0 = (uint32)NULL;

	scheduled_thread->initialized = true;
}
