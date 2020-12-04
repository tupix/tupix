#include <config.h>
#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>
#include <std/io.h>
#include <system/regcheck.h>

bool DEBUG_ENABLED	  = false;
bool SUB_ROUTINE_FLAG = false;

void sub_routine();

void start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();

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

void sub_routine()
{
	SUB_ROUTINE_FLAG = true;
	char c;
	while (1) {
		c = uart_getchar();

		// End sub_routine on EOF
		if (c == 4) {
			SUB_ROUTINE_FLAG = false;
			return;
		}

		for (int i = 0; i < 50; ++i) {
			kprintf("%c", c);
			for (int i = 0; i < BUSY_WAIT_COUNTER; ++i) {}
		}
	}
}
