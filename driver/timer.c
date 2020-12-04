#include <config.h>
#include <std/types.h>
#include <std/util.h>

#define LOCAL_TIMER_BASE 0x40000024

#if 0
#define CORE_INTERRUPT_SOURCE 0x40000060

enum core_interrupt_source_addr {
	CORE_0_INTERRUPT_SOURCE = 0x40000060,
	CORE_1_INTERRUPT_SOURCE = 0x40000064,
	CORE_2_INTERRUPT_SOURCE = 0x40000068,
	CORE_3_INTERRUPT_SOURCE = 0x4000006C,
};

enum interrupt_source_bit_field {
	// 31 - 28 Reserved
	INTERRUPT_SOURCE_PERIHPERAL		 = 12, // Currently not used
	INTERRUPT_SOURCE_LOCAL_TIMER	 = 11,
	INTERRUPT_SOURCE_AXI_OUTSTANDING = 10, // Core 0 only
	INTERRUPT_SOURCE_PMU			 = 9,
	INTERRUPT_SOURCE_GPU			 = 8, // Can be high in one core only
	INTERRUPT_SOURCE_MAILBOX_3		 = 7,
	INTERRUPT_SOURCE_MAILBOX_2		 = 6,
	INTERRUPT_SOURCE_MAILBOX_1		 = 5,
	INTERRUPT_SOURCE_MAILBOX_0		 = 4,
	INTERRUPT_SOURCE_CNTVIRQ		 = 3,
	INTERRUPT_SOURCE_CNTHPIRQ		 = 2,
	INTERRUPT_SOURCE_CNTPNSIRQ		 = 1,
	INTERRUPT_SOURCE_CNTPSIRQ		 = 0, // Physical timer -1
};
#endif

struct local_interrupt {
	uint32 routing;		 // routing
	uint32 padding[3];	 //
	uint32 ctrl_stat;	 // control & status
	uint32 clear_reload; // IRQ clear & reload (write-only)
};

enum local_routing_bit_field {
	// 3 - 31 Unused
	LOCAL_TIMER_ROUTING = 0
};
enum local_timer_routing {
	LOCAL_TIMER_ROUTE_IRQ_0 = 0b000,
	LOCAL_TIMER_ROUTE_IRQ_1 = 0b001,
	LOCAL_TIMER_ROUTE_IRQ_2 = 0b010,
	LOCAL_TIMER_ROUTE_IRQ_3 = 0b011,
	LOCAL_TIMER_ROUTE_FIQ_0 = 0b100,
	LOCAL_TIMER_ROUTE_FIQ_1 = 0b101,
	LOCAL_TIMER_ROUTE_FIQ_2 = 0b110,
	LOCAL_TIMER_ROUTE_FIQ_3 = 0b111,

	// The number of bits these values can occupy
	local_timer_routing_val_len = 3,
};

enum local_timer_control_status {
	LOCAL_TIMER_CTRL_STAT_INTERRUPT_FLAG = 31, // Read-only
	// 30 - Unused
	LOCAL_TIMER_CTRL_STAT_INTERRUPT_ENABLE = 29,
	LOCAL_TIMER_CTRL_STAT_TIMER_ENABLE	   = 28,
	LOCAL_TIMER_CTRL_STAT_RELOAD_VAL	   = 0,

	// Number of bits the reload value occupies
	LOCAL_TIMER_CTRL_STAT_RELOAD_VAL_LEN = 28,
};

// Write-only
enum local_timer_irq_clear_relaod_bit_field {
	LOCAL_TIMER_INTERRUPT_FLAG = 31,
	LOCAL_TIMER_RELOAD		   = 30,
	// 28, 29 not documented?
	// 0 - 27 Unused
};

static volatile struct local_interrupt* const local_interrupt =
		(struct local_interrupt*)LOCAL_TIMER_BASE;

void init_local_timer()
{
	// Enable local timer
	SET_BIT(local_interrupt->ctrl_stat,
			(uint32)LOCAL_TIMER_CTRL_STAT_TIMER_ENABLE);
	// Enable local timer interrupt
	SET_BIT(local_interrupt->ctrl_stat,
			(uint32)LOCAL_TIMER_CTRL_STAT_INTERRUPT_ENABLE);
	// Set reload value
	SET_BIT_TO(local_interrupt->ctrl_stat,
			   (uint32)LOCAL_TIMER_CTRL_STAT_RELOAD_VAL, LOCAL_TIMER_US,
			   (uint32)LOCAL_TIMER_CTRL_STAT_RELOAD_VAL_LEN);
	// Route local timer to core 0
	SET_BIT_TO(local_interrupt->routing, (uint32)LOCAL_TIMER_ROUTING,
			   LOCAL_TIMER_ROUTE_IRQ_0, (uint32)local_timer_routing_val_len);
#if 0
	// Activate local timer in core 0
	static volatile uint32* interrupt_source = (uint32*)CORE_0_INTERRUPT_SOURCE;
	SET_BIT(*interrupt_source, INTERRUPT_SOURCE_LOCAL_TIMER);
#endif
}
