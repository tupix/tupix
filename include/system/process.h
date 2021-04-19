#ifndef SYSTEM_PROCESS_H
#define SYSTEM_PROCESS_H

#include <config.h>

#include <data/types.h>

#include <driver/mmu.h>

#include <system/thread.h>

struct pcb {
	size_t pid, index;
	INDEX_QUEUE(free_indices, N_THREADS_PER_PROCESS);
	__attribute__((aligned(1024))) uint32 l2_table[N_L2_ENTRIES];
};

void process_create(void (*func)(void*), const void* args, size_t args_size,
                    struct registers* regs);

#endif /* SYSTEM_PROCESS_H */
