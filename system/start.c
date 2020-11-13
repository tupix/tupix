#include <config.h>
#include <driver/uart.h>
#include <std/io.h>

void start_kernel(void)
{
	init_uart();

	test_kprintf();

	while (1) {
		char c = kgetchar();
		kprintf("%c %03i %03u %x %p\n", c, c, c, c, c);
	}
}
