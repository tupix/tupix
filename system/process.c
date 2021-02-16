#include <data/types.h>

#include <system/process.h>

static struct pcb processes[MAX_PROCESSES];

static uint64 pid    = 0;
static uint64 pindex = 0;

/**
 * Create a new memory region and start a new thread inside it
 */
void
process_create(void (*func)(void*), const void* args, size_t args_size)
{
	// TODO(Aurel): Set index. Use an index queue and pop.
	size_t index = ++pindex;

	struct pcb* p = &processes[index];
	p->pid        = ++pid; // get incremented pid. pid 0 is reserved
	p->index      = index;
	p->n_threads  = 0; // this also disregards all threads in process->threads
	init_process_memory(p->l2_table);
	klog(DEBUG, "New process %i with l2_table at %p", p->pid, p->l2_table);

	thread_create(p, func, args, args_size);
}
