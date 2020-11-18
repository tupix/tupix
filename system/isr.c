/* ISR - Interrupt Service Routine - Interrupt handler */

/*
 * TODO: All of these might need to be assembler functions, I don't know yet,
 *       but at least the code compiles now
 */

#include <std/io.h>
void reset()
{
    kprintf("reset");
}

void undefined_instruction()
{
    kprintf("undefined instruction");
}

void software_interrupt()
{
    kprintf("software interrupt");
}

void prefetch_abort()
{
    kprintf("prefetch abort");
}

void data_abort()
{
    kprintf("data abort");
}

void not_used()
{
    kprintf("not used");
}

void irq()
{
    kprintf("IRQ");
}

void fiq()
{
    kprintf("FIQ");
}
