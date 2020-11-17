#include <driver/BCM2836.h>
#include <std/types.h>

#define INTERRUPT_REGISTER_BASE (0x7E00B000 - MMU_BASE_OFFSET)

struct interrupt_register {
	uint32 padding0[16]; // Addresses start at 0x200

	uint32 irq_basic_pending; //
	uint64 irq_pending;
	uint32 fiq_control;
	uint64 enable_irqs;
	uint32 enable_basic_irqs;
	uint64 disable_irqs;
	uint32 disable_basic_irqs;
};

// The abbreviation IRQP stands for interrupt request (IRQ) pending
enum irq_basic_pending {
	GPU_IRQ_62 = 20,
	GPU_IRQ_57 = 19,
	GPU_IRQ_56 = 18,
	GPU_IRQ_55 = 17,
	GPU_IRQ_54 = 16,
	GPU_IRQ_53 = 15,
	GPU_IRQ_19 = 14,
	GPU_IRQ_18 = 13,
	GPU_IRQ_10 = 12,
	GPU_IRQ_9  = 11,
	GPU_IRQ_7  = 10,
	IRQ_PEND_GT_0_2 = 8, // One or more bits set in pending register 2
	IRQ_PEND_GT_0_1 = 8, // One or more bits set in pending register 1
	ILL_ACCESS_TYPE_0_IRQP = 7, // Illegal access type 0 IRQ pending
	ILL_ACCESS_TYPE_1_IRQP = 6, // Illegal access type 1 IRQ pending
	GPU1_HALTED_IRQP = 5, // GPU1 halted IRQ pending
	GPU0_HALTED_IRQP = 4, // GPU0 halted IRQ pending
	ARM_DOORBELL_1_IRQP = 3, // ARM Doorbell 1 IRQ pending
	ARM_DOORBELL_0_IRQP = 2, // ARM Doorbell 0 IRQ pending
	ARM_MAILBOX_IRQP = 1, // ARM Mailbox IRQ pending
	ARM_TIMER_IRQP = 0, // ARM Timer IRQ pending
};
// Bit field for irq_pending, enable_irqs and disable_irqs
enum irq_table {
	uart_int = 57,
	pcm_int = 55,
	spi_int = 54,
	i2c_int = 53,
	gpio_int = 49,
	smi = 48,
	pwa1 = 46,
	pwa0 = 45,
	i2c_spi_slv_int = 43,
	aux_int = 29,
};

static volatile uint64* const interrupt_register = (uint64*)INTERRUPT_REGISTER_BASE;
