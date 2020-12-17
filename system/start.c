#include <config.h>

#include <driver/interrupt_controller.h>
#include <driver/timer.h>
#include <driver/uart.h>

#include <system/assert.h>
#include <system/ivt.h>
#include <system/regcheck.h>
#include <system/scheduler.h>
#include <system/thread.h>

#include <std/io.h>
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

	/*
	 * NOTE(Aurel): Simple demonstration of what the scheduler can do as of
	 * right now and how to use it. Note the `init_scheduler` call a few lines up.
	 */
	thread_create(&dummy_run, NULL, 0);
	thread_create(&dummy_run, NULL, 0);
	thread_create(&dummy_run, NULL, 0);
	thread_create(&dummy_run, NULL, 0);

	switch_to_usermode();
}
