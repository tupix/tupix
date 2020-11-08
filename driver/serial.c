#include <serial.h>
#include <stdarg.h>


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b; })

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

char* ultostr(unsigned long n, unsigned int base, char* str, unsigned int* len)
{
	if (base < 2 || base > 36)
		return str; // TODO: ERROR

	// unsigned int num_digits = (int)(log(n) / log(base)) + 1;

	// Start from the least significant digit
	*len = calc_digits(n, base);
	str += *len;
	*str = 0;

	// If n == 0, the loop will be skipped.
	if (!n)
		*str = 0;

	unsigned int cur_digit, ascii_off;
	while (n) {
		cur_digit = n % base;

		if (cur_digit > 9)
			ascii_off = 'a' - 10;
		else
			ascii_off = '0';

		*(--str) = ascii_off + cur_digit;

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
	result = ultostr(n, base, str, len) - offset;
	*len += offset;
	return result;
}

unsigned int str_len(const char* str)
{
	unsigned int len = 0;
	while (*(str++)) {
		len++;
	}
	return len;
}

void kputchar(unsigned char c)
{
	// TODO(aurel): Check if buffer is empty? What else needs to happen to make this safe?
	serial_i->dr = c;
}

void kprint(const char* s)
{
	while (*s) {
		kputchar(*(s++));
	}
}

inline void print_with_padding(const char* num_str, unsigned int len,
							   unsigned int field_width, const char padding)
{
	unsigned int str_len = max(len, field_width);
	for (unsigned int i = 0; i < str_len - len; ++i) {
		kputchar(padding);
	}
	kprint(num_str);
}

void kprintf(const char* format, ...)
{
	const char* cur_char = format;
	unsigned int n_escape = 0;
	while (*cur_char) {
		if (*(cur_char++) != '%')
			continue;

		n_escape++;

		bool repeat = false;
		do {
			switch (*(cur_char++)) {
			case '0' ... '9':
				while (*cur_char >= '0' && *cur_char <= '9') {
					cur_char++;
				}
				repeat = true;
				break;
			case 'x':
			case 'i':
			case 'u':
			case 'p':
			case 's':
			case 'c':
			case '%':
				repeat = false;
				break;
			default:
				// NOTE: Error!
				return;
			}
		} while (repeat);
	}
	// Reset cur_char
	cur_char = format;

	va_list args;
	va_start(args, format);

	char num_str[MAX_NUM_LEN + 3]; // 0x and \0
	unsigned int len;
	while (*cur_char) {
		if (*cur_char != '%') {
			kputchar(*(cur_char++));
			continue;
		}

		char padding = ' ';
		unsigned int field_width = 0;

		bool repeat = false;
		do {
			switch (*(cur_char++)) {
			case '0' ... '9':
				if (*cur_char == '0') {
					padding = '0';
					cur_char++;
				}

				// Move pointer to the end of the number and walk through the number again in
				// reverse.
				const char* beg = cur_char;
				while (*cur_char >= '0' && *cur_char <= '9') {
					++cur_char;
				}
				const char* end = cur_char;
				for (unsigned int power = 1; --cur_char >= beg; power *= 10) {
					unsigned int cur = *cur_char - '0';
					field_width += power * cur;
				}
				cur_char = end;

				repeat = true;
				break;
			case 'c':
				kputchar((unsigned char)va_arg(args, int));
				break;
			case 's': {
				const char* str = va_arg(args, const char*);
				print_with_padding(str, str_len(str), field_width, padding);
				break;
			}
			case 'x':
				ultostr(va_arg(args, unsigned int), 16, num_str, &len);
				print_with_padding(num_str, len, field_width, padding);
				break;
			case 'i':
				ltostr(va_arg(args, int), 10, num_str, &len);
				print_with_padding(num_str, len, field_width, padding);
				break;
			case 'u':
				ultostr(va_arg(args, unsigned int), 10, num_str, &len);
				print_with_padding(num_str, len, field_width, padding);
				break;
			case 'p':
				*num_str = '0';
				*(num_str + 1) = 'x';
				ultostr((unsigned long)va_arg(args, void*), 16, num_str + 2,
						&len);
				print_with_padding(num_str, len, field_width, padding);
				break;
			case '%':
				kputchar('%');
				break;
			}
		} while (repeat);
		cur_char++;
	}

	va_end(args);
}
