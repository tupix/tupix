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

static size_t tid_counter = 0;

extern void exit();           // syscall in user
extern char _ustacks_start[]; // see kernel.lds
extern char _udata_begin[];

#define THREAD_STACK_SIZE 0x400 // 1KB
#define THREAD_STACK_BASE ((void*)_udata_begin)

void*
get_stack_pointer(const size_t index)
{
	ASSERTM(_ustacks_start > (char*)NULL,
	        "_ustacks_start = %p <= 0. This should not "
	        "happen and probably means there is something wrong with your "
	        "linking as this variable should be set to a value > 0 by the "
	        "linker.",
	        _ustacks_start);

	/*
	 * NOTE: All thread stacks are positioned below each other with stack of
	 * thread with index 0 at the very top.
	 */

	// respect max number of threads
	if (index > N_THREADS_PER_PROCESS) {
		klog(ERROR, "Already reached the limit of threads.");
		return NULL;
	}

	// we need to have enough space for the new thread's stack.
	if ((void*)(index * THREAD_STACK_SIZE) > THREAD_STACK_BASE) {
		klog(ERROR, "Not enough memory for this thread's stack.");
		return NULL;
	}

	/*
	 * NOTE(Aurel): mimics index * 2 + 1 + 1 (see
	 * driver/mmu.c:init_thread_memory() for detailed description)
	 */
	void* sp = (void*)(THREAD_STACK_BASE) + index * 0x2000 + 0x1000 + 0x1000 +
			   THREAD_STACK_SIZE;
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
	thread.tid              = ++tid_counter;
	thread.regs.pc          = (uint32)func;
	thread.regs.lr          = (uint32)&exit;
	thread.process          = process;
	thread.cpsr             = PROCESSOR_MODE_USR;
	thread.waiting_duration = 0;

	return thread;
}

struct tcb*
thread_create(struct pcb* p, void (*func)(void*), const void* args,
              size_t args_size, struct registers* regs)
{
	klog(LOG, "Creating new thread...");

	struct tcb new_thread = init_thread(p, func);
	ssize_t index         = pop_index(&(p->free_indices_q));
	if (index < 0) {
		klog(WARNING, "No thread created.");
		return NULL;
	}
	new_thread.index = index;

	init_thread_memory(p->pid, new_thread.index, p->l2_table);
	void* thread_sp = get_stack_pointer(new_thread.index);
	if (!thread_sp) {
		klog(WARNING, "No thread created.");
		return NULL;
	}

	/*
	 * NOTE(Aurel): When creating a new process the l1 entry still points to the
	 * l2 table of the currently running thread. This means we do not have
	 * access to the stack of the thread in creation. That means however, that
	 * we cannot copy over the memory from the old stack to the new stack.
	 */
	if (args && args_size) {
		char args_buffer[args_size];
		memcpy(args_buffer, args, args_size);

		switch_memory(p->l2_table);

		// Since the stack grows to the 'bottom', copy below it
		thread_sp -= args_size;
		memcpy(thread_sp, args_buffer, args_size);

		switch_memory(get_cur_process()->l2_table);
	}

	// Update stack pointer
	new_thread.regs.sp = (uint32)thread_sp;

	// Pass stack-pointer as argument
	if (args && args_size)
		new_thread.regs.r0 = new_thread.regs.sp;
	else
		new_thread.regs.r0 = (uint32)NULL;

	// register thread in scheduler
	struct tcb* scheduled_thread = scheduler_register_thread(&new_thread, regs);

	klog(LOG, "Done creating new thread (p%u,t%u)(pidx%u,tidx%u)",
	     new_thread.process->pid, new_thread.tid, new_thread.process->index,
	     new_thread.index);
	return scheduled_thread;
}
