#include <std/mem.h>

#include <data/types.h>

/*
 * Copies `n` bytes from `src` to `dest`. The memory areas must not overlap.
 * Returns: `dest`
 */
void*
kmemcpy(void* dest, const void* src, const size_t n)
{
	const char* c_src = (const char*)src;
	char* c_dest      = (char*)dest;
	for (size_t i = 0; i < n; ++i)
		c_dest[i] = c_src[i];

	return dest;
}

void*
kmemset(void* mem, const unsigned char val, const size_t n)
{
	unsigned char* c_mem = (unsigned char*)mem;
	for (size_t i = 0; i < n; ++i)
		*(c_mem++) = val;
	return mem;
}
