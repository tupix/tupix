/* ISR - Interrupt Service Routine - Interrupt handler */

/*
 * TODO: All of these might need to be assembler functions, I don't know yet,
 *       but at least the code compiles now
 */

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

// NOTE(Aurel): 8 flags, 3 spaces, 10 chars for the mode name and '\0'
#define PSR_STR_LEN (8 + 3 + 13 + 1)

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
		// TODO(Aurel): This is never supposed to happen. Abort!
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

void print_registers(struct registers reg, char* exc_str,
					 char* exc_system_info_str, char* exc_extra_info_str)
{
	char und_cpsr_str[PSR_STR_LEN];
	char und_spsr_str[PSR_STR_LEN];
	char svc_spsr_str[PSR_STR_LEN];
	char abt_spsr_str[PSR_STR_LEN];
	char fiq_spsr_str[PSR_STR_LEN];
	char irq_spsr_str[PSR_STR_LEN];

	psr_flags_str(reg.cpsr, und_cpsr_str);
	psr_flags_str(reg.spsr, und_spsr_str);
	psr_flags_str(reg.svc_spsr, svc_spsr_str);
	psr_flags_str(reg.abt_spsr, abt_spsr_str);
	psr_flags_str(reg.fiq_spsr, fiq_spsr_str);
	psr_flags_str(reg.irq_spsr, irq_spsr_str);
	psr_flags_str(reg.und_spsr, und_spsr_str);

	// clang-format off
	kprintf("################################################################################\n"
			"%s at address 0x%08x\n"
			"%s\n"
			">>> register snapshot (current mode) <<<\n"
			"R0: 0x%08x	R8:  0x%08x\n"
			"R1: 0x%08x	R9:  0x%08x\n"
			"R2: 0x%08x	R10: 0x%08x\n"
			"R3: 0x%08x	R11: 0x%08x\n"
			"R4: 0x%08x	R12: 0x%08x\n"
			"R5: 0x%08x	SP:  0x%08x\n"
			"R6: 0x%08x	LR:  0x%08x\n"
			"R7: 0x%08x	PC:  0x%08x\n"
			"\n"
			">>> status-register (current mode) <<<\n"
			"CPSR: %s\t(0x%08x)\n"
			"SPSR: %s\t(0x%08x)\n"
			"\n"
			">>> registers (mode-specific) <<<\n"
			"             LR         SP         SPSR\n"
			"User/System: 0x%08x 0x%08x\n"
			"Supervisor:  0x%08x 0x%08x %s\t(0x%08x)\n"
			"Abort:       0x%08x 0x%08x %s\t(0x%08x)\n"
			"FIQ:         0x%08x 0x%08x %s\t(0x%08x)\n"
			"IRQ:         0x%08x 0x%08x %s\t(0x%08x)\n"
			"Undefined:   0x%08x 0x%08x %s\t(0x%08x)\n"
			"\n"
			"%s\n",
			exc_str, reg.lr,
			exc_extra_info_str,
			reg.r0, reg.r8,
			reg.r1, reg.r9,
			reg.r2, reg.r10,
			reg.r3, reg.r11,
			reg.r4, reg.r12,
			reg.r5, reg.sp,
			reg.r6, reg.lr,
			reg.r7, reg.pc,
			und_cpsr_str, reg.cpsr,
			und_spsr_str, reg.spsr,
			reg.usr_lr, reg.usr_sp,
			reg.svc_lr, reg.svc_sp, svc_spsr_str, reg.svc_spsr,
			reg.abt_lr, reg.abt_sp, abt_spsr_str, reg.abt_spsr,
			reg.fiq_lr, reg.fiq_sp, fiq_spsr_str, reg.fiq_spsr,
			reg.irq_lr, reg.irq_sp, irq_spsr_str, reg.irq_spsr,
			reg.und_lr, reg.und_sp, und_spsr_str, reg.spsr,
			exc_system_info_str
		);
	// clang-format on
}

void reset_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Reset", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

void undefined_instruction_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Undefined Instruction", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

void software_interrupt_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Software Interrupt", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

void prefetch_abort_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Prefetch Abort", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

void data_abort_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Data Abort", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

// TODO(Aurel): What has to be done in this case?
void not_used_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	//print_registers(*reg, "Unused", "System halted.", "");
	// TODO(Aurel): Does anything have to happen?
}

void irq_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Interrupt Request", "Continuing.", "");
	// TODO(Aurel): Does anything have to happen?
}

void fiq_handler(void* sp)
{
	volatile struct registers* reg = (struct registers*)sp;
	print_registers(*reg, "Fast Interrupt Request", "Continuing.", "");
	// TODO(Aurel): Does anything have to happen?
}
