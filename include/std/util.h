#ifndef STD_UTIL_H
#define STD_UTIL_H

#include <std/types.h>

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

// TODO: Error handling
#define SET_BIT(bitfield, bit)                                                 \
	do {                                                                       \
		__typeof__(bit) _bit = (bit);                                          \
		if (_bit > (sizeof(__typeof__(bitfield)) * 8) - 1)                     \
			break;                                                             \
		(bitfield) |= 1U << _bit;                                              \
	} while (0)

// TODO: Error handling
#define CLEAR_BIT(bitfield, bit)                                               \
	do {                                                                       \
		__typeof__(bit) _bit = (bit);                                          \
		if (_bit > (sizeof(__typeof__(bitfield)) * 8) - 1)                     \
			break;                                                             \
		(bitfield) &= ~(1U << _bit);                                           \
	} while (0)

// TODO: Error handling
#define SET_BIT_TO(bitfield, bit, value)                                       \
	do {                                                                       \
		__typeof__(bit) _bit	 = (bit);                                      \
		__typeof__(value) _value = (value);                                    \
		if (_bit > (sizeof(__typeof__(bitfield)) * 8) - 1)                     \
			break;                                                             \
		(bitfield) |= _value << _bit;                                          \
	} while (0)

#define IS_SET(bitfield, bit) (!!((bitfield) & (1U << (bit))))

unsigned int calc_digits(unsigned long n, unsigned int base);

char* ultostr(unsigned long n, unsigned int base, char* str, unsigned int* len);
char* ltostr(long n, unsigned int base, char* str, unsigned int* len);

#endif /* STD_UTIL_H */
