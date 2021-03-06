#include <data/types.h>

size_t
kstrlen(const char* str)
{
	size_t len = 0;
	while (*(str++))
		len++;
	return len;
}

char*
kstrncpy(char* dest, const char* src, uint32 n)
{
	uint32 i;
	for (i = 0; i < n && src[i] != '\0'; ++i)
		dest[i] = src[i];
	for (; i < n; ++i)
		dest[i] = '\0';

	return dest;
}
