#include <stdarg.h>

#include <sys/calls.h>

#include <data/types.h>
#include <std/strings.h>
#include <std/util.h>

#define MAX_NUM_LEN 20 // len("-9223372036854775808")

static void
print(const char* str)
{
	while (*str)
		putchar(*(str++));
}

/**
 * Make sure you do not underflow field_width or len when calling.
 */
static void
print_with_padding(const char* num_str, size_t len, size_t field_width,
                   const char padding)
{
	size_t strlen = max(len, field_width);
	for (size_t i = 0; i < strlen - len; ++i)
		putchar(padding);
	print(num_str);
}

static bool
check_format_str(const char* str)
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
			case 'c':
			case '%':
				if (repeat)
					return false;
				break;
			case 'x':
			case 'd':
			case 'i':
			case 'u':
			case 'p':
			case 's':
				repeat = false;
				break;
			default:
				return false;
			}
		} while (repeat);
	}
	return true;
}

static size_t
calc_field_width(const char* cur_char, char* padding, size_t* flag_len)
{
	*flag_len          = 0;
	size_t field_width = 0;

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
		for (uint32 cur, power = 1; --cur_char >= beg; power *= 10) {
			cur = *cur_char - '0';
			field_width += power * cur;
		}
	}
	return field_width;
}

void
printf(const char* format, ...)
{
	if (!check_format_str(format))
		return; // TODO: Error

	va_list args;
	va_start(args, format);

	char padding         = ' ';
	const char* cur_char = format;
	char num_str[MAX_NUM_LEN + 1]; /* MAX_NUM_LEN + len('\0') */
	size_t flags_len, field_width, len;
	while (*cur_char) {
		if (*(cur_char++) != '%') {
			putchar(*(cur_char - 1));
			continue;
		}

		field_width = calc_field_width(cur_char, &padding, &flags_len);
		// Skip flags and jump to conversion specifier
		cur_char += flags_len;

		switch (*cur_char) {
		case 'c':
			putchar((unsigned char)va_arg(args, int));
			break;
		case 's': {
			const char* str = va_arg(args, const char*);
			print_with_padding(str, strlen(str), field_width, padding);
			break;
		}
		case 'x':
			utostr(va_arg(args, uint32), 16, num_str, &len);
			print_with_padding(num_str, len, field_width, padding);
			break;
		case 'd':
		case 'i': {
			itostr(va_arg(args, int32), 10, num_str, &len);
			uint32 offset = 0;
			if (num_str[0] == '-') {
				putchar('-');
				offset = 1;
			}
			print_with_padding(num_str + offset, len - offset,
			                   max((int32)(field_width - offset), 0), padding);
			break;
		}
		case 'u':
			utostr(va_arg(args, uint32), 10, num_str, &len);
			print_with_padding(num_str, len, field_width, padding);
			break;
		case 'p':
			print("0x");
			utostr((uint32)va_arg(args, void*), 16, num_str, &len);
			print_with_padding(num_str, len, field_width, padding);
			break;
		case '%':
			putchar('%');
			break;
		}
		++cur_char;
	}
	va_end(args);
}
