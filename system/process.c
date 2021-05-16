#include <data/types.h>

#include <driver/mmu.h>

#include <system/process.h>
#include <system/scheduler.h>

#include <std/mem.h>

static size_t pid_counter = 0;

extern char _udata_begin[], _udata_end[], _udata_cpy_begin[];
#define UDATA_SIZE _udata_end - _udata_begin

/**
 * Create a new memory region and start a new thread inside it
 */
void
process_create(void (*func)(void*), const void* args, size_t args_size,
               struct registers* regs)
{
	klog(LOG, "Creating new process...");

	struct pcb p = { 0 };
	p.pid        = ++pid_counter;
	INIT_INDEX_QUEUE(p.free_indices);
	MARK_ALL_FREE(p.free_indices_q);

	struct pcb* process = scheduler_register_process(&p);
	if (!process) {
		klog(LOG, "Something went wrong creating a new process.");
		return;
	}

	init_process_memory(process->index, process->l2_table);

	// copy in the user data
	switch_memory(process->l2_table);
	memcpy(_udata_begin, _udata_cpy_begin, UDATA_SIZE);
	switch_memory(get_cur_process()->l2_table);

	struct tcb* thread = thread_create(process, func, args, args_size, regs);
	if (!thread) {
		klog(LOG, "Something went wrong creating a new thread.");
		return;
	}

	klog(LOG, "Done creating new process. (p%u, pidx%u)", process->pid,
	     process->index);
}
