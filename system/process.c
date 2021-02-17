#include <data/types.h>

#include <system/process.h>
#include <system/scheduler.h>

/**
 * Create a new memory region and start a new thread inside it
 */
void
process_create(void (*func)(void*), const void* args, size_t args_size)
{
	struct pcb p        = { 0 };
	struct pcb* process = schedule_process(p);
	init_process_memory(process->l2_table);

	thread_create(process, func, args, args_size);
}
