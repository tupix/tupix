#include <config.h>

#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>

#include <system/assert.h>
#include <system/ivt.h>
#include <system/scheduler.h>
#include <system/thread.h>

#include <std/mem.h>

bool DEBUG_ENABLED = false;

void
start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();

	ASSERTM(N_THREADS >= 32, "A minimum of 32 threads should be supported.");
	init_scheduler();

	switch_to_usermode();
}
