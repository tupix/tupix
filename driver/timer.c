#include <config.h>
#include <std/types.h>
#include <std/util.h>

#define L_TIMER_BASE 0x40000024
#define L_TIMER_CLOCK_SPEED 38.4

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
	INTERRUPT_SOURCE_L_TIMER	 = 11,
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
	L_TIMER_ROUTING = 0
};
enum local_timer_routing {
	L_TIMER_ROUTE_IRQ_0 = 0b000,
	L_TIMER_ROUTE_IRQ_1 = 0b001,
	L_TIMER_ROUTE_IRQ_2 = 0b010,
	L_TIMER_ROUTE_IRQ_3 = 0b011,
	L_TIMER_ROUTE_FIQ_0 = 0b100,
	L_TIMER_ROUTE_FIQ_1 = 0b101,
	L_TIMER_ROUTE_FIQ_2 = 0b110,
	L_TIMER_ROUTE_FIQ_3 = 0b111,

	// The number of bits these values can occupy
	l_timer_routing_val_len = 3,
};

enum local_timer_control_status {
	L_TIMER_CTRL_STAT_INTERRUPT_FLAG = 31, // Read-only
	// 30 - Unused
	L_TIMER_CTRL_STAT_INTERRUPT_ENABLE = 29,
	L_TIMER_CTRL_STAT_TIMER_ENABLE	   = 28,
	L_TIMER_CTRL_STAT_RELOAD_VAL	   = 0,

	// Number of bits the reload value occupies
	L_TIMER_CTRL_STAT_RELOAD_VAL_LEN = 28,
};

// Write-only
enum local_timer_irq_clear_reload_bit_field {
	L_TIMER_INTERRUPT_FLAG = 31,
	L_TIMER_RELOAD		   = 30,
	// 28, 29 not documented?
	// 0 - 27 Unused
};

static volatile struct local_interrupt* const local_interrupt =
		(struct local_interrupt*)L_TIMER_BASE;

void reset_timer()
{
	SET_BIT(local_interrupt->clear_reload, (uint32)L_TIMER_INTERRUPT_FLAG);
}

bool l_timer_is_interrupting()
{
	return IS_SET(local_interrupt->ctrl_stat, L_TIMER_CTRL_STAT_INTERRUPT_FLAG);
}

void init_local_timer()
{
	// Route local timer to core 0
	SET_BIT_TO(local_interrupt->routing, (uint32)L_TIMER_ROUTING,
			   L_TIMER_ROUTE_IRQ_0, (uint32)l_timer_routing_val_len);
// TODO: SET_BIT_TO does not work.
#if 0
	// Enable local timer
	SET_BIT(local_interrupt->ctrl_stat,
			(uint32)L_TIMER_CTRL_STAT_TIMER_ENABLE);
	// Set reload value
	SET_BIT_TO(local_interrupt->ctrl_stat,
			   (uint32)L_TIMER_CTRL_STAT_RELOAD_VAL, LOCAL_TIMER_US,
			   (uint32)L_TIMER_CTRL_STAT_RELOAD_VAL_LEN);
	// Tell timer about new value
	SET_BIT(local_interrupt->clear_reload, (uint32)L_TIMER_RELOAD);
	// Enable local timer interrupt
	SET_BIT(local_interrupt->ctrl_stat,
			(uint32)L_TIMER_CTRL_STAT_INTERRUPT_ENABLE);
#endif
	// Enable timer, timer interrupt and set reload value in one step
	uint32 val = 0;
	val |= ((uint32)(L_TIMER_CLOCK_SPEED * LOCAL_TIMER_US) & 0x0ffffff);
	SET_BIT(val, (uint32)L_TIMER_CTRL_STAT_TIMER_ENABLE);
	SET_BIT(val, (uint32)L_TIMER_CTRL_STAT_INTERRUPT_ENABLE);
	local_interrupt->ctrl_stat = val;
	// Tell timer about new value
	SET_BIT(local_interrupt->clear_reload, (uint32)L_TIMER_RELOAD);
#if 0
	// Activate local timer in core 0
	static volatile uint32* interrupt_source = (uint32*)CORE_0_INTERRUPT_SOURCE;
	SET_BIT(*interrupt_source, INTERRUPT_SOURCE_L_TIMER);
#endif
}
