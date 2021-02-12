#ifndef SYSTEM_MMU_H
#define SYSTEM_MMU_H

#include <data/types.h>

#define N_L1_ENTRIES 4096

// 16KB aligned. TODO: Do not hardcode
//__attribute__((aligned(16384))) static uint32 l1[N_L1_ENTRIES];

void init_mmu();

struct l2_entry {
	uint32 pages[256]; // 1MB are 256 * 4KB
};

void init_thread_memory(size_t index);
void switch_memory(uint32* l2_table);

#endif /* SYSTEM_MMU_H */
