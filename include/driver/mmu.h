#ifndef SYSTEM_MMU_H
#define SYSTEM_MMU_H

#include <data/types.h>

#define N_L1_ENTRIES 4096
#define N_L2_ENTRIES 256

void init_mmu();
void init_process_memory(uint32 index, uint32* l2_table);
void init_thread_memory(size_t pid, size_t thread_index, uint32* l2_table);
void switch_memory(uint32* l2_table);

#endif /* SYSTEM_MMU_H */
