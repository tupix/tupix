#include <config.h>
#include <driver/interrupt_controller.h>
#include <driver/uart.h>
#include <std/io.h>

void start_kernel(void)
{
	init_interrupt_controller();
	init_uart();

	while (1) {
		char c = uart_getchar();
		kprintf("%c %03i %03u %x\n", c, c, c, c);

		switch (c) {
		case 's':
			// trigger Supervisor Call
			break;
		case 'u':
			// trigger Undefined Instruction
			break;
		case 'p':
			// trigger Prefetch Abort
			break;
		case 'd':
			// trigger Data Abort
			break;
		}
	}
}
