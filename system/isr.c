/* ISR - Interrupt Service Routine - Interrupt handler */

#include <system/isr.h>

#include <arch/armv7/registers.h>

#include <driver/timer.h>
#include <driver/uart.h>

#include <system/calls.h>
#include <system/entry.h>
#include <system/scheduler.h>
#include <system/thread.h>

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
		kstrncpy(str, "User\0", 5);
		break;
	case FIQ:
		kstrncpy(str, "FIQ\0", 4);
		break;
	case IRQ:
		kstrncpy(str, "IRQ\0", 4);
		break;
	case SUPERVISOR:
		kstrncpy(str, "Supervisor\0", 11);
		break;
	case ABORT:
		kstrncpy(str, "Abort\0", 6);
		break;
	case UNDEFINED:
		kstrncpy(str, "Undefined\0", 10);
		break;
	case SYSTEM:
		kstrncpy(str, "System\0", 7);
		break;
	case UNINITIALIZED:
		kstrncpy(str, "Uninitialized\0", 14);
		break;
	default:
		kstrncpy(str, "Unknown\0", 8);
		break;
	}
}

void
print_registers(struct registers* regs, char* exc_str,
                char* exc_system_info_str, char* exc_extra_info_str)
{
	char und_cpsr_str[PSR_STR_LEN];
	char und_spsr_str[PSR_STR_LEN];
	char svc_spsr_str[PSR_STR_LEN];
	char abt_spsr_str[PSR_STR_LEN];
	char fiq_spsr_str[PSR_STR_LEN];
	char irq_spsr_str[PSR_STR_LEN];

	psr_flags_str(regs->cpsr, und_cpsr_str);
	psr_flags_str(regs->spsr, und_spsr_str);
	psr_flags_str(regs->svc.spsr, svc_spsr_str);
	psr_flags_str(regs->abt.spsr, abt_spsr_str);
	psr_flags_str(regs->fiq.spsr, fiq_spsr_str);
	psr_flags_str(regs->irq.spsr, irq_spsr_str);
	psr_flags_str(regs->und.spsr, und_spsr_str);

	kprintf("################################################################################\n");
	kprintf("%s at address 0x%08x\n", exc_str, regs->gr.lr);
	kprintf("%s\n", exc_extra_info_str);
	kprintf(">>> register snapshot (current mode) <<<\n");
	kprintf("R0: 0x%08x\tR8:  0x%08x\n", regs->gr.r0, regs->gr.r8);
	kprintf("R1: 0x%08x\tR9:  0x%08x\n", regs->gr.r1, regs->gr.r9);
	kprintf("R2: 0x%08x\tR10: 0x%08x\n", regs->gr.r2, regs->gr.r10);
	kprintf("R3: 0x%08x\tR11: 0x%08x\n", regs->gr.r3, regs->gr.r11);
	kprintf("R4: 0x%08x\tR12: 0x%08x\n", regs->gr.r4, regs->gr.r12);
	kprintf("R5: 0x%08x\tSP:  0x%08x\n", regs->gr.r5, regs->gr.sp);
	kprintf("R6: 0x%08x\tLR:  0x%08x\n", regs->gr.r6, regs->gr.lr);
	kprintf("R7: 0x%08x\tPC:  0x%08x\n", regs->gr.r7, regs->gr.pc);
	kprintf("\n");
	kprintf(">>> status-register (current mode) <<<\n");
	kprintf("CPSR: %s\t(0x%08x)\n", und_cpsr_str, regs->cpsr);
	kprintf("SPSR: %s\t(0x%08x)\n", und_spsr_str, regs->spsr);
	kprintf("\n");
	kprintf(">>> registers (mode-specific) <<<\n");
	kprintf("             LR         SP         SPSR\n");
	kprintf("User/System: 0x%08x 0x%08x\n", regs->usr_lr, regs->und.sp);
	kprintf("Supervisor:  0x%08x 0x%08x %s\t(0x%08x)\n", regs->svc.lr,
	        regs->svc.sp, svc_spsr_str, regs->svc.spsr);
	kprintf("Abort:       0x%08x 0x%08x %s\t(0x%08x)\n", regs->abt.lr,
	        regs->abt.sp, abt_spsr_str, regs->abt.spsr);
	kprintf("FIQ:         0x%08x 0x%08x %s\t(0x%08x)\n", regs->fiq.lr,
	        regs->fiq.sp, fiq_spsr_str, regs->fiq.spsr);
	kprintf("IRQ:         0x%08x 0x%08x %s\t(0x%08x)\n", regs->irq.lr,
	        regs->irq.sp, irq_spsr_str, regs->irq.spsr);
	kprintf("Undefined:   0x%08x 0x%08x %s\t(0x%08x)\n", regs->und.lr,
	        regs->und.sp, und_spsr_str, regs->und.spsr);
	kprintf("\n");
	kprintf("%s\n", exc_system_info_str);
}

bool
user_interrupted(uint32 spsr)
{
	return (spsr & PSR_BITMASK_PROCESSOR_MODE) == PROCESSOR_MODE_USR;
}

/*
 * Most of the handlers do the same.
 * This macro creates a function that loops endlessly on system interrupt and
 * otherwise kills the current thread. In both cases a register snapshot is
 * printed.
 */
#define HANDLER_FUNCTION(exc_name, exc_name_print)                             \
	void exc_name##_handler(struct registers* regs)                            \
	{                                                                          \
		if (user_interrupted(regs->spsr)) {                                    \
			print_registers(regs, exc_name_print, "Killing thread.", "");      \
			kill_cur_thread(regs);                                             \
		} else {                                                               \
			print_registers(regs, exc_name_print, "System halted.", "");       \
			endless_loop();                                                    \
		}                                                                      \
	}

HANDLER_FUNCTION(undefined_instruction, "Undefined Instruction")
HANDLER_FUNCTION(prefetch_abort, "Prefetch Abort")
HANDLER_FUNCTION(data_abort, "Data Abort")

void
software_interrupt_handler(struct registers* regs)
{
	if (!user_interrupted(regs->spsr)) {
		print_registers(regs, "Software Interrupt", "System halted.", "");
		endless_loop();
		return;
	}

	disable_timer();

	uint16 id = get_syscall_id(regs->gr.lr);
	exec_syscall(id, regs);

	reset_timer();
	enable_timer();
}

void
irq_handler(struct registers* regs)
{
	// Reset triggered interrupts
	if (l_timer_is_interrupting()) {
		kprintf("!");
		scheduler_cycle(regs, true);
		reset_timer();
	} else if (uart_is_interrupting()) {
		if (!uart_push_char()) {
			klog(ERROR, "Could not buffer new char");
			return;
		}

		// Notify scheduler that UART received a char.
		scheduler_on_char_received();

	} else {
		print_registers(regs, "Unknown Interrupt Request (IRQ)", "Continuing.",
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
