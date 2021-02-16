#include <config.h>

#include <driver/interrupt_controller.h>
#include <driver/mmu.h>
#include <driver/timer.h>
#include <driver/uart.h>

#include <system/assert.h>
#include <system/ivt.h>
#include <system/process.h>
#include <system/scheduler.h>

#include <std/mem.h>

extern void main_thread(void*);

void
start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();
	init_mmu();
	klog(LOG, "Kernel initialized.");

	ASSERTM(N_THREADS >= 32, "A minimum of 32 threads should be supported.");
	init_scheduler();

	process_create(&main_thread, NULL, 0);
}
