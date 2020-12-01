#include <std/types.h>

unsigned int str_len(const char* str)
{
	unsigned int len = 0;
	while (*(str++))
		len++;
	return len;
}

char* strncpy(char* dest, const char* src, uint64 n)
{
	uint64 i;
	for (i = 0; i < n && src[i] != '\0'; ++i)
		dest[i] = src[i];
	for (; i < n; ++i)
		dest[i] = '\0';

	return dest;
}
