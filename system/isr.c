/* ISR - Interrupt Service Routine - Interrupt handler */

#include <system/isr.h>

#include <arch/armv7/registers.h>

#include <driver/timer.h>
#include <driver/uart.h>

#include <system/calls.h>
#include <system/entry.h>
#include <system/scheduler.h>
#include <system/thread.h>
#include <system/user_thread.h>

#include <data/types.h>
#include <std/bits.h>
#include <std/io.h>
#include <std/strings.h>
#include <std/util.h>

enum cpsr_mode_bits {
	USER          = 0b10000,
	FIQ           = 0b10001,
	IRQ           = 0b10010,
	SUPERVISOR    = 0b10011,
	ABORT         = 0b10111,
	UNDEFINED     = 0b11011,
	SYSTEM        = 0b11111,
	UNINITIALIZED = 0,
};

enum psr_flags_idx_bitfield {
	PSR_NEGATIVE   = 31,
	PSR_ZERO       = 30,
	PSR_CARRY      = 29,
	PSR_OVERFLOW   = 28,
	PSR_ENDIANESS  = 9,
	PSR_MASK_IRQ   = 7,
	PSR_MASK_FIQ   = 6,
	PSR_THUMB_MODE = 5,
};

// NOTE(Aurel): 8 flags, 3 spaces, 10 chars for the mode name and '\0'
#define PSR_STR_LEN (8 + 3 + 13 + 1)

void
psr_flags_str(uint32 flags, char* str)
{
	// clang-format off
	str[0]  = IS_SET(flags, PSR_NEGATIVE)   ? 'N' : '_';
	str[1]  = IS_SET(flags, PSR_ZERO)       ? 'Z' : '_';
	str[2]  = IS_SET(flags, PSR_CARRY)      ? 'C' : '_';
	str[3]  = IS_SET(flags, PSR_OVERFLOW)   ? 'V' : '_';
	str[4]  = ' ';
	str[5]  = IS_SET(flags, PSR_ENDIANESS)  ? 'E' : '_';
	str[6]  = ' ';
	str[7]  = IS_SET(flags, PSR_MASK_IRQ)   ? 'I' : '_';
	str[8]  = IS_SET(flags, PSR_MASK_FIQ)   ? 'F' : '_';
	str[9]  = IS_SET(flags, PSR_THUMB_MODE) ? 'T' : '_';
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
		strncpy(str, "Unknown\0", 8);
		break;
	}
}

void
print_registers(volatile struct registers* reg, char* exc_str,
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
	psr_flags_str(reg->svc.spsr, svc_spsr_str);
	psr_flags_str(reg->abt.spsr, abt_spsr_str);
	psr_flags_str(reg->fiq.spsr, fiq_spsr_str);
	psr_flags_str(reg->irq.spsr, irq_spsr_str);
	psr_flags_str(reg->und.spsr, und_spsr_str);

	kprintf("################################################################################\n");
	kprintf("%s at address 0x%08x\n", exc_str, reg->gr.lr);
	kprintf("%s\n", exc_extra_info_str);
	kprintf(">>> register snapshot (current mode) <<<\n");
	kprintf("R0: 0x%08x\tR8:  0x%08x\n", reg->gr.r0, reg->gr.r8);
	kprintf("R1: 0x%08x\tR9:  0x%08x\n", reg->gr.r1, reg->gr.r9);
	kprintf("R2: 0x%08x\tR10: 0x%08x\n", reg->gr.r2, reg->gr.r10);
	kprintf("R3: 0x%08x\tR11: 0x%08x\n", reg->gr.r3, reg->gr.r11);
	kprintf("R4: 0x%08x\tR12: 0x%08x\n", reg->gr.r4, reg->gr.r12);
	kprintf("R5: 0x%08x\tSP:  0x%08x\n", reg->gr.r5, reg->gr.sp);
	kprintf("R6: 0x%08x\tLR:  0x%08x\n", reg->gr.r6, reg->gr.lr);
	kprintf("R7: 0x%08x\tPC:  0x%08x\n", reg->gr.r7, reg->gr.pc);
	kprintf("\n");
	kprintf(">>> status-register (current mode) <<<\n");
	kprintf("CPSR: %s\t(0x%08x)\n", und_cpsr_str, reg->cpsr);
	kprintf("SPSR: %s\t(0x%08x)\n", und_spsr_str, reg->spsr);
	kprintf("\n");
	kprintf(">>> registers (mode-specific) <<<\n");
	kprintf("             LR         SP         SPSR\n");
	kprintf("User/System: 0x%08x 0x%08x\n", reg->usr_lr, reg->und.sp);
	kprintf("Supervisor:  0x%08x 0x%08x %s\t(0x%08x)\n", reg->svc.lr,
	        reg->svc.sp, svc_spsr_str, reg->svc.spsr);
	kprintf("Abort:       0x%08x 0x%08x %s\t(0x%08x)\n", reg->abt.lr,
	        reg->abt.sp, abt_spsr_str, reg->abt.spsr);
	kprintf("FIQ:         0x%08x 0x%08x %s\t(0x%08x)\n", reg->fiq.lr,
	        reg->fiq.sp, fiq_spsr_str, reg->fiq.spsr);
	kprintf("IRQ:         0x%08x 0x%08x %s\t(0x%08x)\n", reg->irq.lr,
	        reg->irq.sp, irq_spsr_str, reg->irq.spsr);
	kprintf("Undefined:   0x%08x 0x%08x %s\t(0x%08x)\n", reg->und.lr,
	        reg->und.sp, und_spsr_str, reg->und.spsr);
	kprintf("\n");
	kprintf("%s\n", exc_system_info_str);
}

bool
user_interrupted(uint32 spsr)
{
	return (spsr & PSR_BITMASK_PROCESSOR_MODE) == PROCESSOR_MODE_USR;
}

void
undefined_instruction_handler(volatile struct registers* reg)
{
	if (user_interrupted(reg->spsr)) {
		print_registers(reg, "Undefined Instruction", "Killing thread.", "");
	} else {
		endless_loop();
		print_registers(reg, "Undefined Instruction", "System halted.", "");
	}
}

void
software_interrupt_handler(volatile struct registers* reg)
{
	if (!user_interrupted(reg->spsr)) {
		print_registers(reg, "Software Interrupt", "System halted.", "");
		endless_loop();
	} else if (get_syscall_id(reg->gr.lr) != 1) {
		// Syscall-id 1 means that the thread returned. Then we want no register
		// snapshot.
		print_registers(reg, "Software Interrupt", "Killing thread.", "");
	}
	log(LOG, "Syscall with id %i called.", get_syscall_id(reg->gr.lr));
}

void
prefetch_abort_handler(volatile struct registers* reg)
{
	if (user_interrupted(reg->spsr)) {
		print_registers(reg, "Prefetch Abort", "Killing thread.", "");
	} else {
		print_registers(reg, "Prefetch Abort", "System halted.", "");
		endless_loop();
	}
}

void
data_abort_handler(volatile struct registers* reg)
{
	if (user_interrupted(reg->spsr)) {
		print_registers(reg, "Data Abort", "Killing thread.", "");
	} else {
		print_registers(reg, "Data Abort", "System halted.", "");
		endless_loop();
	}
}

void
irq_handler(volatile struct registers* reg)
{
	// Reset triggered interrupts
	if (l_timer_is_interrupting()) {
		kprintf("!");
		scheduler_cycle((struct registers*)reg); // Discard volatile
		reset_timer();
	} else if (uart_is_interrupting()) {
		if (!uart_push_char()) {
			log(ERROR, "Could not buffer new char");
			return;
		}
		char c = uart_pop_char();
		log(LOG, "Pressed %c 0x%02x %i ", c, c, c);
		switch (c) {
		case 'S':
			trigger_exception(SUPERVISOR_CALL);
			break;
		case 'P':
			trigger_exception(PREFETCH_ABORT);
			break;
		case 'A':
			trigger_exception(DATA_ABORT);
			break;
		case 'U':
			trigger_exception(UNDEFINED_INSTRUCTION);
			break;
		default:
			break;
		}

		uint32 aligned_c = (uint32)c;
		thread_create(&user_thread, &aligned_c, sizeof(aligned_c));
	} else {
		print_registers(reg, "Unknown Interrupt Request (IRQ)", "Continuing.",
		                "");
	}
}

void
trigger_exception(enum exception exc)
{
	switch (exc) {
	case SUPERVISOR_CALL:
		asm("svc #0");
		break;
	case PREFETCH_ABORT:
		asm("bkpt");
		break;
	case DATA_ABORT:
		// TODO
		break;
	case UNDEFINED_INSTRUCTION:
		asm("udf");
		break;
	default:
		break;
	}
}
