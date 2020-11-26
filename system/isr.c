/* ISR - Interrupt Service Routine - Interrupt handler */

/*
 * TODO: All of these might need to be assembler functions, I don't know yet,
 *       but at least the code compiles now
 */

#include <std/io.h>
#include <std/types.h>

struct registers {
	uint32 r0;
	uint32 r1;
	uint32 r2;
	uint32 r3;
	uint32 r4;
	uint32 r5;
	uint32 r6;
	uint32 r7;
	uint32 r8;
	uint32 r9;
	uint32 r10;
	uint32 r11;
	uint32 r12;
	uint32 sp;
	uint32 lr;
	uint32 pc;

#if 0
	uint32 cpsr;
	uint32 spsr;

	uint32 usr_sp;
	uint32 usr_lr;
	uint32 usr_pc;

	uint32 svc_sp;
	uint32 svc_lr;
	uint32 svc_pc;

	uint32 und_sp;
	uint32 und_lr;
	uint32 und_pc;

	uint32 abt_sp;
	uint32 abt_lr;
	uint32 abt_pc;

	uint32 irq_sp;
	uint32 irq_lr;
	uint32 irq_pc;

	uint32 fiq_sp;
	uint32 fiq_lr;
	uint32 fiq_pc;
#endif
};

void print_registers(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
}

void reset()
{
	kprintf("reset\n");
}

#if 0
void undefined_instruction()
{
	kprintf("undefined instruction\n");
}
#endif

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
