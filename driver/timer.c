#include <config.h>

#include <data/types.h>

#include <std/bits.h>
#include <std/util.h>

#define L_TIMER_BASE 0x40000024
#define L_TIMER_CLOCK_SPEED 38.4

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
	SET_BIT(local_interrupt->clear_reload, L_TIMER_INTERRUPT_FLAG);
}

bool l_timer_is_interrupting()
{
	return IS_SET(local_interrupt->ctrl_stat, L_TIMER_CTRL_STAT_INTERRUPT_FLAG);
}

void init_local_timer()
{
	// Route local timer to core 0
	SET_BIT_TO(local_interrupt->routing, L_TIMER_ROUTING, L_TIMER_ROUTE_IRQ_0,
			   l_timer_routing_val_len);
	uint32 val = 0;
	// Enable local timer
	SET_BIT(val, L_TIMER_CTRL_STAT_TIMER_ENABLE);
	// Set reload value
	SET_BIT_TO(val, L_TIMER_CTRL_STAT_RELOAD_VAL,
			   (uint32)(L_TIMER_CLOCK_SPEED * LOCAL_TIMER_US),
			   L_TIMER_CTRL_STAT_RELOAD_VAL_LEN);
	// Enable local timer interrupt
	SET_BIT(val, L_TIMER_CTRL_STAT_INTERRUPT_ENABLE);
	local_interrupt->ctrl_stat = val;
	// Tell timer about new value
	SET_BIT(local_interrupt->clear_reload, L_TIMER_RELOAD);
}
