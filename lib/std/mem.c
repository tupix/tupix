#include <std/mem.h>

#include <data/types.h>

/*
 * Copies `n` bytes from `src` to `dest`. The memory areas must not overlap.
 * Returns: `dest`
 */
void*
memcpy(const void* src, void* dest, uint32 n)
{
	char* c_src	 = (char*)src;
	char* c_dest = (char*)dest;
	for (uint32 i = 0; i < n; ++i)
		c_dest[i] = c_src[i];

	return dest;
}

void*
memset(void* mem, const unsigned char val, const size_t n)
{
	unsigned char* c_mem = (unsigned char*)mem;
	for (size_t i = 0; i < n; ++i)
		*(c_mem++) = val;
	return mem;
}
