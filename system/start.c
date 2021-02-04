#include <config.h>

#include <driver/interrupt_controller.h>
#include <driver/mmu.h>
#include <driver/timer.h>
#include <driver/uart.h>

#include <system/assert.h>
#include <system/ivt.h>
#include <system/scheduler.h>
#include <system/thread.h>

#include <std/mem.h>

extern void user_thread(void*);

void
start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();
	//init_mmu();

	ASSERTM(N_THREADS >= 32, "A minimum of 32 threads should be supported.");
	init_scheduler();

	thread_create(&user_thread, NULL, 0);
	switch_to_usermode();
}
