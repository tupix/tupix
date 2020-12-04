#include <config.h>
#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>
#include <std/io.h>
#include <system/isr.h>
#include <system/regcheck.h>

void sub_routine();

void start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();

	char c;
	while (1) {
		c = uart_getchar();
		kprintf("%c %03i %03u %x\n", c, c, c, c); // TODO: remove, only debug
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
			// trigger Data Abort
			// TODO: trigger
			break;
		case 'e':
			sub_routine();
			SUB_ROUTINE_FLAG = 0;
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
	SUB_ROUTINE_FLAG = 1;
	char c;
	while (1) {
		c = uart_getchar();
		// End sub_routine on EOF
		if (c == 4)
			return;
		for (int i = 0; i < 50; ++i) {
			kprintf("%c", c);
			for (int i = 0; i < BUSY_WAIT_COUNTER; ++i) {}
		}
	}
}
