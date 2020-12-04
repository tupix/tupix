#include "std/util.h"

#include <data/types.h>

unsigned int calc_digits(unsigned long n, unsigned int base)
{
	// Start at 1 when n == 0
	unsigned int num = !n;

	while (n) {
		n /= base;
		num++;
	}
	return num;
}

char* ultostr(unsigned long n, unsigned int base, char* str, unsigned int* len)
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

	unsigned int cur_digit, ascii_offset;
	while (n) {
		cur_digit = n % base;

		// ASCII digits and letters are not consecutive to each other which is why we have to differentiate
		// between them and take a different offset.
		if (cur_digit >= 10) {
			// a is the 10th hex-digit
			cur_digit -= 10;
			ascii_offset = 'a';
		} else {
			ascii_offset = '0';
		}
		*(--str) = cur_digit + ascii_offset;

		n /= base;
	}
	return str;
}

char* ltostr(long n, unsigned int base, char* str, unsigned int* len)
{
	char* result;
	unsigned int offset = 0;
	if (n < 0) {
		*(str++) = '-';
		n *= -1;
		offset = 1;
	}

	// returns str + offset
	result = ultostr(n, base, str, len) - offset;
	*len += offset;

	return result;
}
