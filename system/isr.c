/* ISR - Interrupt Service Routine - Interrupt handler */

#include <driver/timer.h>
#include <driver/uart.h>
#include <std/io.h>
#include <std/strings.h>
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

enum psr_flags_idx_bitfield {
	PSR_NEGATIVE   = 31,
	PSR_ZERO	   = 30,
	PSR_CARRY	   = 29,
	PSR_OVERFLOW   = 28,
	PSR_ENDIANESS  = 9,
	PSR_MASK_IRQ   = 7,
	PSR_MASK_FIQ   = 6,
	PSR_THUMB_MODE = 5,
};

// NOTE(Aurel): 8 flags, 3 spaces, 10 chars for the mode name and '\0'
#define PSR_STR_LEN (8 + 3 + 13 + 1)

void psr_flags_str(uint32 flags, char* str)
{
	// clang-format off
	str[0]	= IS_SET(flags, PSR_NEGATIVE)   ? 'N' : '_';
	str[1]	= IS_SET(flags, PSR_ZERO)       ? 'Z' : '_';
	str[2]	= IS_SET(flags, PSR_CARRY)      ? 'C' : '_';
	str[3]	= IS_SET(flags, PSR_OVERFLOW)   ? 'V' : '_';
	str[4]	= ' ';
	str[5]	= IS_SET(flags, PSR_ENDIANESS)  ? 'E' : '_';
	str[6]	= ' ';
	str[7]	= IS_SET(flags, PSR_MASK_IRQ)   ? 'I' : '_';
	str[8]	= IS_SET(flags, PSR_MASK_FIQ)   ? 'F' : '_';
	str[9]	= IS_SET(flags, PSR_THUMB_MODE) ? 'T' : '_';
	str[10] = ' ';
	// clang-format on
	str += 11;

	switch ((enum cpsr_mode_bits)(flags & 0b11111)) {
	case USER:
		strncpy(str, "User\0", 5);
		break;
	case FIQ:
		strncpy(str, "FIQ\0", 4);
		break;
	case IRQ:
		strncpy(str, "IRQ\0", 4);
		break;
	case SUPERVISOR:
		strncpy(str, "Supervisor\0", 11);
		break;
	case ABORT:
		strncpy(str, "Abort\0", 6);
		break;
	case UNDEFINED:
		strncpy(str, "Undefined\0", 10);
		break;
	case SYSTEM:
		strncpy(str, "System\0", 7);
		break;
	case UNINITIALIZED:
		strncpy(str, "Uninitialized\0", 14);
		break;
	default:
		// TODO(Aurel): This should never happen. Assert!
		strncpy(str, "Unknown\0", 8);
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

	uint32 und_lr;
	uint32 und_sp;
	uint32 und_spsr;

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

void print_registers(volatile struct registers* reg, char* exc_str,
					 char* exc_system_info_str, char* exc_extra_info_str)
{
	char und_cpsr_str[PSR_STR_LEN];
	char und_spsr_str[PSR_STR_LEN];
	char svc_spsr_str[PSR_STR_LEN];
	char abt_spsr_str[PSR_STR_LEN];
	char fiq_spsr_str[PSR_STR_LEN];
	char irq_spsr_str[PSR_STR_LEN];

	psr_flags_str(reg->cpsr, und_cpsr_str);
	psr_flags_str(reg->spsr, und_spsr_str);
	psr_flags_str(reg->svc_spsr, svc_spsr_str);
	psr_flags_str(reg->abt_spsr, abt_spsr_str);
	psr_flags_str(reg->fiq_spsr, fiq_spsr_str);
	psr_flags_str(reg->irq_spsr, irq_spsr_str);
	psr_flags_str(reg->und_spsr, und_spsr_str);

	kprintf("################################################################################\n");
	kprintf("%s at address 0x%08x\n", exc_str, reg->lr);
	kprintf("%s\n", exc_extra_info_str);
	kprintf(">>> register snapshot (current mode) <<<\n");
	kprintf("R0: 0x%08x\tR8:  0x%08x\n", reg->r0, reg->r8);
	kprintf("R1: 0x%08x\tR9:  0x%08x\n", reg->r1, reg->r9);
	kprintf("R2: 0x%08x\tR10: 0x%08x\n", reg->r2, reg->r10);
	kprintf("R3: 0x%08x\tR11: 0x%08x\n", reg->r3, reg->r11);
	kprintf("R4: 0x%08x\tR12: 0x%08x\n", reg->r4, reg->r12);
	kprintf("R5: 0x%08x\tSP:  0x%08x\n", reg->r5, reg->sp);
	kprintf("R6: 0x%08x\tLR:  0x%08x\n", reg->r6, reg->lr);
	kprintf("R7: 0x%08x\tPC:  0x%08x\n", reg->r7, reg->pc);
	kprintf("\n");
	kprintf(">>> status-register (current mode) <<<\n");
	kprintf("CPSR: %s\t(0x%08x)\n", und_cpsr_str, reg->cpsr);
	kprintf("SPSR: %s\t(0x%08x)\n", und_spsr_str, reg->spsr);
	kprintf("\n");
	kprintf(">>> registers (mode-specific) <<<\n");
	kprintf("             LR         SP         SPSR\n");
	kprintf("User/System: 0x%08x 0x%08x\n", reg->usr_lr, reg->und_sp);
	kprintf("Supervisor:  0x%08x 0x%08x %s\t(0x%08x)\n", reg->svc_lr,
			reg->svc_sp, svc_spsr_str, reg->svc_spsr);
	kprintf("Abort:       0x%08x 0x%08x %s\t(0x%08x)\n", reg->abt_lr,
			reg->abt_sp, abt_spsr_str, reg->abt_spsr);
	kprintf("FIQ:         0x%08x 0x%08x %s\t(0x%08x)\n", reg->fiq_lr,
			reg->fiq_sp, fiq_spsr_str, reg->fiq_spsr);
	kprintf("IRQ:         0x%08x 0x%08x %s\t(0x%08x)\n", reg->irq_lr,
			reg->irq_sp, irq_spsr_str, reg->irq_spsr);
	kprintf("Undefined:   0x%08x 0x%08x %s\t(0x%08x)\n", reg->und_lr,
			reg->und_sp, und_spsr_str, reg->und_spsr);
	kprintf("\n");
	kprintf("%s\n", exc_system_info_str);
}

void undefined_instruction_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(reg, "Undefined Instruction", "System halted.", "");
	while (true) {}
}

void software_interrupt_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(reg, "Software Interrupt", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

void prefetch_abort_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(reg, "Prefetch Abort", "System halted.", "");
	while (true) {}
}

void data_abort_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(reg, "Data Abort", "System halted.", "");
	while (true) {}
}

void irq_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;

	// Reset triggered interrupts
	if (l_timer_is_interrupting()) {
		kprintf("T");
		reset_timer();
	} else if (uart_is_interrupting()) {
		if (uart_buffer_char() == -1)
			return;

		// TODO(Aurel): Usage code. Move somewhere else.
		char c;
		if (uart_getchar(&c) == 0)
			kprintf("%c\n", c);

	} else {
		print_registers(reg, "Unknown Interrupt Request (IRQ)", "Continuing.",
						"");
	}
}

void fiq_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(reg, "Fast Interrupt Request", "Continuing.", "");
	// TODO(Aurel): Does anything have to happen?
}
