/* ISR - Interrupt Service Routine - Interrupt handler */

/*
 * TODO: All of these might need to be assembler functions, I don't know yet,
 *       but at least the code compiles now
 */

#include <std/io.h>
#include <std/types.h>
#include <std/util.h>

enum cpsr_mode_bits {
	USER		  = 0b10000,
	FIQ			  = 0b10001,
	IRQ			  = 0b10010,
	SUPERVISOR	  = 0b10011,
	ABORT		  = 0b10111,
	UNDEFINED	  = 0b11011,
	SYSTEM		  = 0b11111,
	UNINITIALIZED = 0,
};

void psr_flags_str(uint32 flags, char* str)
{
	str[4]	= ' ';
	str[6]	= ' ';
	str[10] = ' ';

	str[0] = IS_SET(flags, 31) ? 'N' : '_';
	str[1] = IS_SET(flags, 30) ? 'Z' : '_';
	str[2] = IS_SET(flags, 29) ? 'C' : '_';
	str[3] = IS_SET(flags, 28) ? 'V' : '_';

	str[5] = '_';

	str[7] = IS_SET(flags, 7) ? 'I' : '_';
	str[8] = IS_SET(flags, 6) ? 'F' : '_';
	str[9] = IS_SET(flags, 5) ? 'T' : '_';

	// TODO(aurel): This is not working? Why? It puts the correct string into
	// mode_str, but that does not write them into str..?
	char* mode_str = str + 10;
	switch ((enum cpsr_mode_bits)(flags & 0b11111)) {
	case USER:
		mode_str = "User\0";
		break;
	case FIQ:
		mode_str = "FIQ\0";
		break;
	case IRQ:
		mode_str = "IRQ\0";
		break;
	case SUPERVISOR:
		mode_str = "Supervisor\0";
		break;
	case ABORT:
		mode_str = "Abort\0";
		break;
	case UNDEFINED:
		mode_str = "Undefined\0";
		break;
	case SYSTEM:
		mode_str = "System\0";
		break;
	case UNINITIALIZED:
		mode_str = "Uninitialized\0";
        break;
	}
}

struct registers {
	uint32 fiq_lr;
	uint32 fiq_sp;
	uint32 fiq_spsr;

	uint32 irq_lr;
	uint32 irq_sp;
	uint32 irq_spsr;

	uint32 abt_lr;
	uint32 abt_sp;
	uint32 abt_spsr;

	uint32 svc_lr;
	uint32 svc_sp;
	uint32 svc_spsr;

	uint32 usr_lr;
	uint32 usr_sp;

	uint32 cpsr;
	uint32 spsr;

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

	uint32 lr;
	uint32 pc;
	uint32 sp;
};

/*
 * NOTE(Aurel): ATTENTION. This function only works with undefined exception
 * right now
 * TODO(Aurel): Make usable for all kinds of exceptions
 */
void print_registers(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;

	// NOTE(Aurel): 8 flags, 3 spaces, 10 chars for the mode name and '\0'
	char und_cpsr_str[8 + 3 + 10 + 1];
	char und_spsr_str[8 + 3 + 10 + 1];
	char svc_spsr_str[8 + 3 + 10 + 1];
	char abt_spsr_str[8 + 3 + 10 + 1];
	char fiq_spsr_str[8 + 3 + 10 + 1];
	char irq_spsr_str[8 + 3 + 10 + 1];

	psr_flags_str(reg->cpsr, und_cpsr_str);
	psr_flags_str(reg->spsr, und_spsr_str);
	psr_flags_str(reg->svc_spsr, svc_spsr_str);
	psr_flags_str(reg->abt_spsr, abt_spsr_str);
	psr_flags_str(reg->fiq_spsr, fiq_spsr_str);
	psr_flags_str(reg->irq_spsr, irq_spsr_str);

	// clang-format off
	kprintf("################################################################################\n"
			"Undefined Instruction at address 0x%08x\n"
			"\n"
			">>> register snapshot (current mode) <<<\n"
			"R0: 0x%08x    R8:  0x%08x\n"
			"R1: 0x%08x    R9:  0x%08x\n"
			"R2: 0x%08x    R10: 0x%08x\n"
			"R3: 0x%08x    R11: 0x%08x\n"
			"R4: 0x%08x    R12: 0x%08x\n"
			"R5: 0x%08x    SP:  0x%08x\n"
			"R6: 0x%08x    LR:  0x%08x\n"
			"R7: 0x%08x    PC:  0x%08x\n"
			"\n"
			">>> current status-register <<<\n"
			"CPSR: %s\t(0x%08x)\n"
			"SPSR: %s\t(0x%08x)\n"
			"\n"
			">>> current mode-specific registers <<<\n"
			"             LR         SP         SPSR\n"
			"User/System: 0x%08x 0x%08x\n" /* TODO(Aurel): Why is there no space for the SPSR here? */
            "Supervisor:  0x%08x 0x%08x %s\t(0x%08x)\n"
			"Abort:       0x%08x 0x%08x %s\t(0x%08x)\n"
			"FIQ:         0x%08x 0x%08x %s\t(0x%08x)\n"
			"IRQ:         0x%08x 0x%08x %s\t(0x%08x)\n"
			"Undefined:   0x%08x 0x%08x %s\t(0x%08x)\n"
			"\n"
			"System halted.\n",
			reg->lr - 8, // TODO(Aurel): I don't think this is always 8.
			reg->r0, reg->r8,
            reg->r1, reg->r9,
            reg->r2, reg->r10,
            reg->r3, reg->r11,
            reg->r4, reg->r12,
            reg->r5, reg->sp,
            reg->r6, reg->lr,
			reg->r7, reg->pc,
            und_cpsr_str, reg->cpsr,
			und_spsr_str, reg->spsr,
            reg->usr_lr, reg->usr_sp,
			reg->svc_lr, reg->svc_sp, svc_spsr_str, reg->svc_spsr,
			reg->abt_lr, reg->abt_sp, abt_spsr_str, reg->abt_spsr,
			reg->fiq_lr, reg->fiq_sp, fiq_spsr_str, reg->fiq_spsr,
			reg->irq_lr, reg->irq_sp, irq_spsr_str, reg->irq_spsr,
			// NOTE(Aurel): The following is only because we are handling undefined instruction exception.
			reg->lr, reg->sp, und_spsr_str, reg->spsr);
	// clang-format on
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
