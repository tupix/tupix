#include <data/types.h>

#include <driver/mmu.h>

#include <system/process.h>
#include <system/scheduler.h>

#include <std/mem.h>

extern char _udata_begin[], _udata_end[], _udata_cpy_begin[];
#define UDATA_SIZE _udata_end - _udata_begin

/*
 * TODO(Aurel): Index queues to place the threads in the processes list. This
 * index is then also used for the memory.
 */

/**
 * Create a new memory region and start a new thread inside it
 */
void
process_create(void (*func)(void*), const void* args, size_t args_size)
{
	klog(LOG, "Creating new process...");
	struct pcb p        = { 0 };
	struct pcb* process = schedule_process(p);
	if (!process) {
		klog(LOG, "Something went wrong creating a new thread.");
		return;
	}

	init_process_memory(process->l2_table);

	// copy in the user data
	switch_memory(process->pid, process->l2_table);

	memcpy(_udata_begin, _udata_cpy_begin, UDATA_SIZE);

	// TODO(Aurel): Cleanup into own function?
	struct pcb* cur_process = get_cur_process();
	switch_memory(cur_process->pid, cur_process->l2_table);

	thread_create(process, func, args, args_size);
	klog(LOG, "Done creating new process. (p%u)", process->pid);
}
