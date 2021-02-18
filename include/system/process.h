#ifndef SYSTEM_PROCESS_H
#define SYSTEM_PROCESS_H

#include <data/types.h>

#include <system/thread.h>

struct pcb {
	size_t pid, index, n_threads;
	struct tcb threads[N_THREADS];
	__attribute__((aligned(1024))) uint32 l2_table[256];
};

void process_create(void (*func)(void*), const void* args, size_t args_size);

#endif /* SYSTEM_PROCESS_H */
