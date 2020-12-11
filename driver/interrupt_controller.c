#include <driver/BCM2836.h>

#include <data/types.h>
#include <std/bits.h>
#include <std/util.h>

#define INTERRUPT_REGISTER_BASE (0x7E00B000 - MMU_BASE_OFFSET)

struct interrupt_register {
	uint32 irq_basic_pending; // which interrupts are pending
	uint32 irq_pending_l;
	uint32 irq_pending_h;

	/* NOTE: Only a single bit can be selected in FIQ control register. */
	uint32 fiq_control; // which interrupt source can generate a FIQ.

	uint32 enable_irq_l;
	uint32 enable_irq_h;
	uint32 enable_basic_irqs; // enable an interrupt source
	uint32 disable_irq_l;
	uint32 disable_irq_h;
	uint32 disable_basic_irqs; // disable an interrupt source
};

// NOTE(Aurel): The abbreviation IRQP stands for interrupt request (IRQ) pending
enum irq_basic_pending {
	GPU_IRQ_62			   = 20,
	GPU_IRQ_57			   = 19,
	GPU_IRQ_56			   = 18,
	GPU_IRQ_55			   = 17,
	GPU_IRQ_54			   = 16,
	GPU_IRQ_53			   = 15,
	GPU_IRQ_19			   = 14,
	GPU_IRQ_18			   = 13,
	GPU_IRQ_10			   = 12,
	GPU_IRQ_9			   = 11,
	GPU_IRQ_7			   = 10,
	IRQ_PEND_GT_0_2		   = 8, // One or more bits set in pending register 2
	IRQ_PEND_GT_0_1		   = 8, // One or more bits set in pending register 1
	ILL_ACCESS_TYPE_0_IRQP = 7, // Illegal access type 0 IRQ pending
	ILL_ACCESS_TYPE_1_IRQP = 6, // Illegal access type 1 IRQ pending
	GPU1_HALTED_IRQP	   = 5, // GPU1 halted IRQ pending
	GPU0_HALTED_IRQP	   = 4, // GPU0 halted IRQ pending
	ARM_DOORBELL_1_IRQP	   = 3, // ARM Doorbell 1 IRQ pending
	ARM_DOORBELL_0_IRQP	   = 2, // ARM Doorbell 0 IRQ pending
	ARM_MAILBOX_IRQP	   = 1, // ARM Mailbox IRQ pending
	ARM_TIMER_IRQP		   = 0, // ARM Timer IRQ pending
};

// Bit field for irq_pending, enable_irqs and disable_irqs
enum irq_table {
	IRQ_UART_INT = 57, // UART
};

enum fiq_control_bit_field {
	FIQ_CONTROL_ENABLE = 7, // enable the FIQ generation
	FIQ_CONTROL_SOURCE = 0, // select FIQ source
};

static volatile struct interrupt_register* const ir =
		(struct interrupt_register*)(INTERRUPT_REGISTER_BASE + 0x200);

void
init_interrupt_controller()
{
	CLEAR_BIT(ir->disable_irq_h, IRQ_UART_INT - 32);
	SET_BIT(ir->enable_irq_h, IRQ_UART_INT - 32);
}
