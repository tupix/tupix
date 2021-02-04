#include <driver/mmu.h>

#include <system/assert.h>

#include <std/bits.h>
#include <std/log.h>

extern uint32 _l1_start[N_L1_ENTRIES];
#define L1 _l1_start

void
init_l1()
{
	ASSERTM(1 == 0, "Not implemented yet.");
}

uint32
get_dacr_init_val(uint32 dacr)
{
	// TODO(Aurel): Do we need to set any domain to a specific value?
	// Domain 0 -> client mode?
	ASSERTM(1 == 0, "Not implemented yet.");
	return dacr;
}

uint32
get_ttbcr_init_val(uint32 ttbcr)
{
	// TODO(Aurel): Init ttbcr.
	ASSERTM(1 == 0, "Not implemented yet.");
	return ttbcr;
}
