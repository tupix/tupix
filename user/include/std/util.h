#ifndef STD_UTIL_H
#define STD_UTIL_H

#include <data/types.h>

// NOTE: Taken from StackOverflow:
// [see](https://stackoverflow.com/questions/3437404/min-and-max-in-c)
#define max(a, b)                                                              \
	({                                                                         \
		__typeof__(a) _a = (a);                                                \
		__typeof__(b) _b = (b);                                                \
		_a > _b ? _a : _b;                                                     \
	})

#define min(a, b)                                                              \
	({                                                                         \
		__typeof__(a) _a = (a);                                                \
		__typeof__(b) _b = (b);                                                \
		_a < _b ? _a : _b;                                                     \
	})

uint32 calc_digits(uint32 n, uint32 base);

char* utostr(uint32 n, uint8 base, char* str, size_t* len);
char* itostr(int32 n, uint8 base, char* str, size_t* len);

#endif /* STD_UTIL_H */
