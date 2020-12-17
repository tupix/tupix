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

#if 0
bool SUB_ROUTINE_FLAG = false;

#define BUSY_WAIT_N_CHARS 50
void
sub_routine()
{
	SUB_ROUTINE_FLAG = true;
	char c;
	while (1) {
		c = uart_getchar();

		for (int i = 0; i < BUSY_WAIT_N_CHARS; ++i) {
			kprintf("%c", c);
			// We need a volatile counter so that the loop is not optimized out.
			for (volatile int j = 0; j < BUSY_WAIT_COUNTER; ++j) {}
		}
	}
}
#endif

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
#if 0
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
#endif
}
