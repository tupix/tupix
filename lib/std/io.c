#include <stdarg.h>

#include <driver/uart.h>

#include <std/strings.h>
#include <std/types.h>
#include <std/util.h>

#define MAX_NUM_LEN 20 // len("-9223372036854775808")

void kprint(const char* s)
{
	while (*s)
		uart_putchar(*(s++));
}

inline void print_with_padding(const char* num_str, unsigned int len,
							   unsigned int field_width, const char padding)
{
	unsigned int str_len = max(len, field_width);
	for (unsigned int i = 0; i < str_len - len; ++i)
		uart_putchar(padding);
	kprint(num_str);
}

bool check_format_str(const char* str)
{
	const char* cur_char = str;
	while (*cur_char) {
		if (*(cur_char++) != '%')
			continue;

		bool repeat = false;
		do {
			switch (*(cur_char++)) {
			case '0' ... '9':
				while (*cur_char >= '0' && *cur_char <= '9')
					++cur_char;
				repeat = true;
				break;
			case 'x':
			case 'i':
			case 'u':
			case 'p':
			case 's':
			case 'c': // TODO: check if repeat == true and error?
			case '%':
				repeat = false;
				break;
			default:
				// NOTE: Error!
				return false;
			}
		} while (repeat);
	}
	return true;
}

unsigned int calc_field_width(const char* cur_char, char* padding,
							  unsigned int* flag_len)
{
	*flag_len				 = 0;
	unsigned int field_width = 0;

	if (*cur_char >= '0' && *cur_char <= '9') {
		if (*cur_char == '0') {
			*padding = '0';
			++cur_char;
			++(*flag_len);
		} else {
			*padding = ' ';
		}

		// Move pointer to the end of the number and walk through the number again in reverse.
		const char* beg = cur_char;
		while (*cur_char >= '0' && *cur_char <= '9') {
			++cur_char;
			++(*flag_len);
		}
		for (unsigned int cur, power = 1; --cur_char >= beg; power *= 10) {
			cur = *cur_char - '0';
			field_width += power * cur;
		}
	}
	return field_width;
}

void kprintf(const char* format, ...)
{
	if (!check_format_str(format))
		return;

	va_list args;
	va_start(args, format);

	char padding;
	const char* cur_char = format;
	char num_str[MAX_NUM_LEN + 1]; /* MAX_NUM_LEN + len('\0') */
	unsigned int flags_len, field_width, len;
	while (*cur_char) {
		if (*(cur_char++) != '%') {
			uart_putchar(*(cur_char - 1));
			continue;
		}

		field_width = calc_field_width(cur_char, &padding, &flags_len);
		// Skip flags and jump to conversion specifier
		cur_char += flags_len;

		switch (*cur_char) {
		case 'c':
			uart_putchar((unsigned char)va_arg(args, int));
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
		case 'i': {
			ltostr(va_arg(args, int), 10, num_str, &len);
			int offset = 0;
			if (num_str[0] == '-') {
				uart_putchar('-');
				offset = 1;
			}
			print_with_padding(num_str + offset, len - offset,
							   field_width - offset, padding);
			break;
		}
		case 'u':
			ultostr(va_arg(args, unsigned int), 10, num_str, &len);
			print_with_padding(num_str, len, field_width, padding);
			break;
		case 'p':
			kprint("0x");
			ultostr((unsigned long)va_arg(args, void*), 16, num_str, &len);
			print_with_padding(num_str, len, field_width, padding);
			break;
		case '%':
			uart_putchar('%');
			break;
		}
		++cur_char;
	}
	va_end(args);
}
