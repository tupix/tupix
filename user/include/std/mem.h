#ifndef STD_MEM_H
#define STD_MEM_H

#include <data/types.h>

#define NULL ((void*)0)

/*
 * Copies `n` bytes from `src` to `dest`. The memory areas must not overlap.
 * Returns: `dest`
 */
void* umemcpy(void* dest, const void* src, const size_t n);

/*
 * Sets `n` bytes to `val` at address `mem`.
 * Returns: `mem`
 */
void* umemset(void* mem, const unsigned char val, const size_t n);

// TODO: memmove()

#endif /* STD_MEM_H */
