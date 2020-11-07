#include <stdarg.h>

// TODO(aurel): Move this to a util library!!
enum bool { true = 1, false = 0 };
typedef enum bool bool;

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

void kputchar(char c)
{
	// TODO(aurel): Check if buffer is empty? What else needs to happen to make this safe?
	serial_i->dr = c;
}

void kprintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	const char* cur_char = format;
	unsigned int count = 0;
	while (*cur_char) {
		if (*(cur_char++) != '%')
			continue;

		switch (*cur_char) {
		case 'c':
		case 's':
		case 'x':
		case 'i':
		case 'u':
		case 'p':
			count++;
			// Intentional fall through
		case '%':
			break;
		default:
			// NOTE: Error!
			return;
		}
		cur_char++;
	}

	char* string[count];

	while (*cur_char) {
		if (*cur_char != '%') {
			kputchar(*cur_char);
			cur_char++;
			continue;
		}
		switch (*(++cur_char)) {
		case 'c':
			break;
		case 's':
			break;
		case 'x':
			break;
		case 'i':
			break;
		case 'u':
			break;
		case 'p':
			break;
		case '%':
			kputchar('%')
			break;
		default:
			// NOTE: Error!
			// Not possible since we checked in first loop
			return;
		}
		cur_char++;
	}
}
