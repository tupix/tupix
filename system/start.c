#include <config.h>
#include <driver/uart.h>
#include <std/io.h>

void start_kernel(void)
{
	init_uart();

	while (1) {
		char c = uart_getchar();
		kprintf("%c %03i %03u %x\n", c, c, c, c);
	}
}
