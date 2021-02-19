#ifndef SYSTEM_PROCESS_H
#define SYSTEM_PROCESS_H

#include <config.h>

#include <data/types.h>

#include <system/thread.h>

struct pcb {
	size_t pid, index;
	INDEX_QUEUE(free_indices, N_THREADS_PER_PROCESS);
	__attribute__((aligned(1024))) uint32 l2_table[256];
};

void process_create(void (*func)(void*), const void* args, size_t args_size);

#endif /* SYSTEM_PROCESS_H */
