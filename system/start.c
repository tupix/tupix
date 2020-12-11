#include <config.h>

#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>

#include <system/assert.h>
#include <system/regcheck.h>

#include <std/io.h>

bool DEBUG_ENABLED	  = false;
bool SUB_ROUTINE_FLAG = false;

#define BUSY_WAIT_N_CHARS 50
void
sub_routine()
{
	SUB_ROUTINE_FLAG = true;
	char c;
	while (1) {
		c = uart_getchar();

		for (int i = 0; i < BUSY_WAIT_N_CHARS; ++i) {
			kprintf("%c", c);
			// We need a volatile counter so that the loop is not optimized out.
			for (volatile int j = 0; j < BUSY_WAIT_COUNTER; ++j) {}
		}
	}
}

void
start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();

	ASSERTM(N_THREADS >= 32, "A minimum of 32 threads should be supported.");
	char c;
	while (1) {
		c = uart_getchar();
		switch (c) {
		case 's':
			// trigger Supervisor Call
			asm("svc #0");
			break;
		case 'u':
			// trigger Undefined Instruction
			asm("udf");
			break;
		case 'p':
			// trigger Prefetch Abort
			asm("bkpt");
			break;
		case 'd':
			DEBUG_ENABLED = !DEBUG_ENABLED;
			break;
		case 'e':
			sub_routine();
			break;
		case 'c':
			register_checker();
			break;
		default:
			// Do nothing
			break;
		}
	}
}
