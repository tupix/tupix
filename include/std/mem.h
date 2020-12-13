#ifndef STD_MEM_H
#define STD_MEM_H

#include <data/types.h>

#define NULL 0x0

void* memcpy(const void* src, void* dest, uint32 n);
void* memset(void* mem, const unsigned char val, const size_t n); // TODO: TEST!
// TODO: memmove()

#endif /* STD_MEM_H */
