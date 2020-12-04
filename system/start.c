#include <config.h>
#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>
#include <std/io.h>

void start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();
}
