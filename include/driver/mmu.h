#ifndef SYSTEM_MMU_H
#define SYSTEM_MMU_H

#include <data/types.h>

#define N_L1_ENTRIES 4096

// 16KB aligned. TODO: Do not hardcode
__attribute__ ((aligned (16384)))static uint32 l1[N_L1_ENTRIES];

void init_mmu();

#endif /* SYSTEM_MMU_H */
