#include <sys/calls.h>

void
user_thread(void* args)
{
	if (!args)
		while (1)
			putchar('f');
}

#if 0
#define PRINT_N 10
void
user_thread(void* x)
{
	uint32 c = 'f';
	if (x) {
		c = *(uint32*)x;
		x += sizeof(c);
		switch (c) {
		case 's':
			trigger_exception(SUPERVISOR_CALL);
			break;
		case 'p':
			trigger_exception(PREFETCH_ABORT);
			break;
		case 'a':
			trigger_exception(DATA_ABORT);
			break;
		case 'u':
			trigger_exception(UNDEFINED_INSTRUCTION);
			break;
		case 'c':
			register_checker();
			break;
		default:
			break;
		}
	}
	for (size_t i = 0; i < PRINT_N; ++i) {
		kprintf("%c", c);
		// We need a volatile counter so that the loop is not optimized out.
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER * 10; ++i) {}
	}
	return;
}
#endif
