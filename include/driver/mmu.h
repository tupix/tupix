#ifndef SYSTEM_MMU_H
#define SYSTEM_MMU_H

#include <data/types.h>

#define N_L1_ENTRIES 4096

// 16KB aligned. TODO: Do not hardcode
//__attribute__((aligned(16384))) static uint32 l1[N_L1_ENTRIES];

void init_mmu();
void init_process_memory(uint32* l2_table);
void init_thread_memory(size_t pid, size_t thread_index, uint32* l2_table);
void switch_memory(uint32 pid, uint32* l2_table);

#endif /* SYSTEM_MMU_H */
