#ifndef STD_MEM_H
#define STD_MEM_H

#include <data/types.h>

#define NULL ((void*)0)

void* umemcpy(void* dest, const void* src, const size_t n);
void* umemset(void* mem, const unsigned char val, const size_t n);
// TODO: memmove()

#endif /* STD_MEM_H */
