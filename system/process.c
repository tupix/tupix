#include <data/types.h>

#include <system/process.h>
#include <system/scheduler.h>

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

	thread_create(process, func, args, args_size);
	klog(LOG, "Done creating new process. (p%u)", process->pid);
}
