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
			asm("svc #0");
			break;
		case 'u':
			// trigger Undefined Instruction
			asm(".word 0xf7f0a000\n");
			break;
		case 'p':
			// trigger Prefetch Abort
			asm("bkpt #0");
			break;
		case 'd':
			// trigger Data Abort
			// TODO: trigger
			break;
		}
	}
}
