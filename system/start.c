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

extern char _udata_begin[], _udata_end[], _udata_cpy_begin[];
#define UDATA_SIZE _udata_end - _udata_begin

void
start_kernel(void)
{
	init_interrupt_controller();
	init_uart();
	init_local_timer();
	init_mmu();
	klog(LOG, "Kernel initialized.");

	ASSERTM(UDATA_SIZE < 0x1000, "user data is bigger than reserved memory.");
	// Copy user data and bss segments to kernel memory.
	memcpy(_udata_cpy_begin, _udata_begin, UDATA_SIZE);

	ASSERTM(N_THREADS >= 32, "A minimum of 32 threads should be supported.");
	init_scheduler();

	process_create(&main_thread, NULL, 0, NULL);
}
