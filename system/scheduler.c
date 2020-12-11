#include <config.h>

#include <data/types.h>
#include <system/thread.h>

struct queue_tcb {
	struct tcb tcb;
	struct queue_tcb* prev;
	struct queue_tcb* next;
};

struct queue_tcb thread_blocks[N_THREADS];

void
thread_create(void (*func)(void*), const void* args, uint32 args_size)
{
}
