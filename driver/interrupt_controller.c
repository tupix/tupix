#include <driver/BCM2836.h>

#include <std/types.h>
#include <std/util.h>

#define INTERRUPT_REGISTER_BASE (0x7E00B000 - MMU_BASE_OFFSET)

struct interrupt_register {
	uint32 padding0[16]; // Addresses start at 0x200

	uint32 irq_basic_pending; // which interrupts are pending
	uint64 irq_pending;		  // which interrupts are pending in detail

	/* NOTE: Only a single bit can be selected in FIQ control register. */
	uint32 fiq_control; // which interrupt source can generate a FIQ.

	uint64 enable_irqs;		   // enable an interrupt source
	uint32 enable_basic_irqs;  // enable an interrupt source
	uint64 disable_irqs;	   // disable an interrupt source
	uint32 disable_basic_irqs; // disable an interrupt source
};

// The abbreviation IRQP stands for interrupt request (IRQ) pending
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
	irq_uart_int		= 57, // UART
	irq_pcm_int			= 55,
	irq_spi_int			= 54,
	irq_i2c_int			= 53,
	irq_gpio_int		= 49,
	irq_smi				= 48,
	irq_pwa1			= 46,
	irq_pwa0			= 45,
	irq_i2c_spi_slv_int = 43,
	irq_aux_int			= 29,
};

enum fiq_table {
	fiq_ill_access_type_0 = 71,
	fiq_ill_access_type_1 = 70,
	fiq_gpu1_halted		  = 69,
	fiq_gpu0_halted		  = 68,
	fiq_arm_doorbell_1	  = 67,
	fiq_arm_doorbell_0	  = 66,
	fiq_arm_mailbox		  = 65,
	fiq_arm_timer		  = 64,
	fiq_uart_int		  = 57, // UART
	fiq_pcm_int			  = 55,
	fiq_spi_int			  = 54,
	fiq_i2c_int			  = 53,
	fiq_gpio_int		  = 49,
	fiq_smi				  = 48,
	fiq_pwa1			  = 46,
	fiq_pwa0			  = 45,
	fiq_i2c_spi_slv_int	  = 43,
	fiq_aux_int			  = 29,
};

enum fiq_control_bit_field {
	FIQ_CONTROL_ENABLE = 7, // enable the FIQ generation
	FIQ_CONTROL_SOURCE = 0, // select FIQ source
};

static volatile struct interrupt_register* const ir =
		(struct interrupt_register*)INTERRUPT_REGISTER_BASE;

void init_interrupt_controller()
{
	// TODO: This is for the fast interrupt
	//set_bit(&(ir->fiq_control), FIQ_CONTROL_ENABLE);
	//ir->fiq_control |= fiq_arm_timer; // FIQ should be Timer interrupt

	CLEAR_BIT(ir->disable_irqs, (uint32)irq_uart_int);
	SET_BIT(ir->enable_irqs, (uint32)irq_uart_int);
}
