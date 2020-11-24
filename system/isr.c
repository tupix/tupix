/* ISR - Interrupt Service Routine - Interrupt handler */

/*
 * TODO: All of these might need to be assembler functions, I don't know yet,
 *       but at least the code compiles now
 */

#include <std/io.h>
void reset()
{
	kprintf("reset\n");
}

void undefined_instruction()
{
	kprintf("undefined instruction\n");
}

void software_interrupt()
{
	kprintf("software interrupt\n");
}

void prefetch_abort()
{
	kprintf("prefetch abort\n");
}

void data_abort()
{
	kprintf("data abort\n");
}

void not_used()
{
	kprintf("not used\n");
}

void irq()
{
	kprintf("IRQ\n");
}

void fiq()
{
	kprintf("FIQ\n");
}
