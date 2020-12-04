#include <config.h>
#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>
#include <std/io.h>

void start_kernel(void)
{
	init_interrupt_controller();
	init_local_timer();
	init_uart();

#if 0
	while (1) {
		char c = uart_getchar();
		kprintf("%c %03i %03u %x\n", c, c, c, c);

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
		}
	}
#endif
}
