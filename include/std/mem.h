#ifndef STD_MEM_H
#define STD_MEM_H

#include <data/types.h>

#define NULL ((void*)0)

void* memcpy(void* dest, const void* src, uint32 n);
void* memset(void* mem, const unsigned char val, const size_t n); // TODO: TEST!
// TODO: memmove()

#endif /* STD_MEM_H */
