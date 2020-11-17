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
