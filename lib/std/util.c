#include "std/util.h"

#include <data/types.h>

uint32 calc_digits(uint32 n, uint32 base)
{
	// Start at 1 when n == 0
	uint32 num = !n;

	while (n) {
		n /= base;
		num++;
	}
	return num;
}

char* utostr(uint32 n, uint8 base, char* str, size_t* len)
{
	if (base < 2 || base > 36)
		return str; // TODO: ERROR

	// Start from the least significant digit
	*len = calc_digits(n, base);
	str += *len;
	*str = '\0';

	// If n == 0, just return "0\0"
	if (!n) {
		*(--str) = '0';
		return str;
	}

	size_t cur_digit;
	char ascii_offset;
	while (n) {
		cur_digit = n % base;

		// ASCII digits and letters are not consecutive to each other which is why we have to differentiate
		// between them and take a different offset.
		if (cur_digit >= 0xa) {
			cur_digit -= 0xa;
			ascii_offset = 'a';
		} else {
			ascii_offset = '0';
		}
		*(--str) = cur_digit + ascii_offset;

		n /= base;
	}
	return str;
}

char* itostr(int32 n, uint8 base, char* str, size_t* len)
{
	size_t offset = 0;
	if (n < 0) {
		*(str++) = '-';
		n *= -1;
		offset = 1;
	}

	// returns str + offset
	char* result = utostr(n, base, str, len) - offset;
	*len += offset;

	return result;
}
