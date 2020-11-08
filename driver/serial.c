#include <serial.h>
#include <stdarg.h>

// TODO(aurel): Move this to a util library!!
typedef enum bool { true = 1, false = 0 } bool;

#define SERIAL_BASE (0x7E201000 - 0x3F000000)
#define MAX_NUM_LEN 20 // len(-9223372036854775808)

struct serial {
	unsigned int dr; // data register
	unsigned int rsrecr;
	unsigned int padding0[4];
	unsigned int fr; // flag register
	unsigned int padding1;
	unsigned int ilpr;
	unsigned int ibrd;
	unsigned int fbrd;
	unsigned int lcrh;
	unsigned int cr; // control register
	unsigned int ifls;
	unsigned int imsc;
	unsigned int ris;
	unsigned int mis;
	unsigned int icr;
	unsigned int dmacr;
	unsigned int padding2[13];
	unsigned int itcr;
	unsigned int itip;
	unsigned int itop;
	unsigned int tdr;
};

static volatile struct serial* const serial_i = (struct serial*)SERIAL_BASE;

unsigned int calc_digits(unsigned int n, unsigned int base)
{
	// Start at 1 when n == 0
	unsigned int num = !n;

	while (n) {
		n /= base;
		num++;
	}
	return num;
}

char* ltostr(long n, unsigned int base, char* str)
{
	if (n < 0) {
		*str++ = '-';
		n *= -1;
	}
	return ultostr(n, base, str) - 1;
}

char* ultostr(unsigned long n, unsigned int base, char* str)
{
	if (base < 2 || base > 36)
		return str; // TODO: ERROR

	// unsigned int num_digits = (int)(log(n) / log(base)) + 1;

	// Start from the least significant digit
	str += calc_digits(n, base);
	*str = 0;

	// If n == 0, the loop will be skipped.
	if (!n)
		*str = 0;

	unsigned int cur_digit, ascii_off;
	while (n) {
		cur_digit = n % base;

		if (cur_digit > 9)
			ascii_off = 87; // ASCII a - 10
		else
			ascii_off = 48; // ASCII 0

		*--str = ascii_off + cur_digit;

		n /= base;
	}

	return str;
}

void kputchar(unsigned char c)
{
	// TODO(aurel): Check if buffer is empty? What else needs to happen to make this safe?
	serial_i->dr = c;
}

void kputs(const char* s)
{
	while (*s) {
		kputchar(*(s++));
	}
}

void kprintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	const char* cur_char = format;
	while (*cur_char) {
		if (*(cur_char++) != '%')
			continue;

		switch (*(cur_char++)) {
		case 'c':
		case 's':
		case 'x':
		case 'i':
		case 'u':
		case 'p':
		case '%':
			break;
		default:
			// NOTE: Error!
			return;
		}
	}
	// Reset cur_char
	cur_char = format;

	char num_str[MAX_NUM_LEN + 3]; // 0x and \0
	while (*cur_char) {
		if (*cur_char != '%') {
			kputchar(*(cur_char++));
			continue;
		}

		switch (*(++cur_char)) {
		case 'c':
			kputchar((unsigned char)va_arg(args, int));
			break;
		case 's':
			kputs(va_arg(args, const char*));
			break;
		case 'x':
			kputs(ultostr(va_arg(args, unsigned int), 16, num_str));
			break;
		case 'i':
			kputs(ltostr(va_arg(args, int), 10, num_str));
			break;
		case 'u':
			kputs(ultostr(va_arg(args, unsigned int), 10, num_str));
			break;
		case 'p':
			*num_str = '0';
			*(num_str + 1) = 'x';
			kputs(ultostr((unsigned long)va_arg(args, void*), 16, num_str + 2) - 2);
			break;
		case '%':
			kputchar('%');
			break;
		}
		cur_char++;
	}

	va_end(args);
}
