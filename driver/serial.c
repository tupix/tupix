#include <stdarg.h>

// TODO(aurel): Move this to a util library!!
typedef enum bool { true = 1, false = 0 } bool;

#define SERIAL_BASE (0x7E201000 - 0x3F000000)

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
			break;
		case 'i':
			break;
		case 'u':
			break;
		case 'p':
			// uintptr_t
			break;
		case '%':
			kputchar('%');
			break;
		}
		cur_char++;
	}

	va_end(args);
}
