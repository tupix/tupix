#include <config.h>

#include <system/thread.h>

#include <arch/armv7/registers.h>

#include <driver/mmu.h>

#include <data/types.h>

#include <system/assert.h>
#include <system/ivt.h>
#include <system/process.h>
#include <system/scheduler.h>

#include <std/log.h>
#include <std/mem.h>

extern void exit();           // syscall in user
extern char _ustacks_start[]; // see kernel.lds

#define THREAD_STACK_SIZE 0x400 // 1KB
// NOTE(Aurel): Place at the beginning of the second 4KB page
#define THREAD_STACK_BASE ((void*)_ustacks_start)

void*
get_stack_pointer(const size_t index)
{
	ASSERTM(_ustacks_start > (char*)NULL,
	        "_ustacks_start = %p <= 0. This should not "
	        "happen and probably means there is something wrong with your "
	        "linking as this variable should be set to a value > 0 by the "
	        "linker.",
	        _ustacks_start);

	// All thread stacks are positioned on top of each other with stack for id 0
	// at the very top.

	// Respect max number of threads
	if (index >= N_THREADS) {
		klog(ERROR, "Already reached the limit of threads.");
		return NULL;
	}

	// We need to have enough space for the new thread's stack.
	if ((void*)(index * THREAD_STACK_SIZE) > THREAD_STACK_BASE) {
		klog(ERROR, "Not enough memory for this thread's stack.");
		return NULL;
	}

	// TODO(Aurel): Currently, no more than one stack per process can exist.
	// stacks get allocated downwards in memory
	// mimics index * 2 + 1 (see system/mmu.c:init_thread_memory())
	void* sp = (void*)(THREAD_STACK_BASE) + index * 0x2000 + 0x1000 +
			   THREAD_STACK_SIZE;
	klog(DEBUG, "sp: %p", sp);
	return sp;
}

void*
get_max_stack_pointer(const size_t index)
{
	return get_stack_pointer(index) - THREAD_STACK_SIZE;
}

struct tcb
init_thread(struct pcb* process, void (*func)(void*))
{
	struct tcb thread       = { 0 };
	thread.regs.pc          = (uint32)func;
	thread.regs.lr          = (uint32)&exit;
	thread.process          = process;
	thread.cpsr             = PROCESSOR_MODE_USR;
	thread.waiting_duration = 0;
	thread.initialized      = false;

	return thread;
}

void
thread_create(struct pcb* p, void (*func)(void*), const void* args,
              size_t args_size)
{
	struct tcb* scheduled_thread = schedule_thread(init_thread(p, func));
	if (!scheduled_thread)
		return; // Thread was not added to queue

	void* thread_sp = get_stack_pointer(scheduled_thread->index);

	// TODO(Aurel): Error handling for thread_sp == NULL

	// Since the stack grows to the 'bottom', copy below it
	// TODO: What to do if we cannot access user memory?
	if (args && args_size) {
		thread_sp -= args_size;
		memcpy(thread_sp, args, args_size);
	}

	// Update stack pointer
	scheduled_thread->regs.sp = (uint32)thread_sp;

	init_thread_memory(p->pid, scheduled_thread->index, p->l2_table);

	// Pass stack-pointer as argument
	if (args && args_size)
		scheduled_thread->regs.r0 = scheduled_thread->regs.sp;
	else
		scheduled_thread->regs.r0 = (uint32)NULL;

	p->n_threads++;
	scheduled_thread->initialized = true;
}
