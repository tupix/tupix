#include <driver/mmu.h>

#include <system/assert.h>

#include <std/bits.h>
#include <std/log.h>

#define KB *1024
#define MB KB * 1024

extern uint32 _l1_start[N_L1_ENTRIES];
#define L1 _l1_start

enum l1_access_permission {
	L1_ACCESS_PERM_SYS_USER_FULL           = 0b011,
	L1_ACCESS_PERM_SYS_FULL_USER_READ_ONLY = 0b010,
	L1_ACCESS_PERM_SYS_USER_READ_ONLY      = 0b111,
	L1_ACCESS_PERM_SYS_ONLY_READ_ONLY      = 0b101,
	L1_ACCESS_PERM_SYS_ONLY_FULL           = 0b001,
	L1_ACCESS_PERM_NO                      = 0b000,
};

enum l1_bit_field {
	L1_SECTION_BASE_ADDRESS = 20, // Physical address to map to
	L1_NS                   = 19, //
	L1_0                    = 18, //
	L1_nG                   = 17, // not Global -> process specific
	L1_S                    = 16, //
	L1_AP_2                 = 15, // Access Permission[2]
	L1_TEX                  = 12, //
	L1_AP_0                 = 10, // Access Permission[1:0]
	L1_IMPL                 = 9,  //
	L1_DOMAIN               = 5,  //
	L1_XN                   = 4,  // eXecute Never
	L1_C                    = 3,  //
	L1_B                    = 2,  //
	L1_1                    = 1,  //
	L1_PXN                  = 0,  // Privileged eXecute Never
};

uint32
set_l1_access_permission(uint32 l1_entry, enum l1_access_permission permission)
{
	/*
	 * NOTE(Aurel): Permission bits are split into two:
	 * L1_AP_0 takes the first two bits
	 * L1_AP_2 takes the third bit
	 */
	SET_BIT_TO(l1_entry, L1_AP_0, permission, 2);
	SET_BIT_TO(l1_entry, L1_AP_2, (permission >> 2), 1);
	return l1_entry;
}

uint32
set_base_address_of_index(uint32 index)
{
	// TODO(Aurel): Checks for the size. What's its biggest size?
	return (index MB)&0xfff00000;
}

void
init_l1()
{
	ASSERTM(1 == 0, "Not implemented yet.");
	// TODO(Aurel): This is an example for the first index. Should actually be
	// right, I think.
	L1[0] = set_l1_access_permission(set_base_address_of_index(0),
	                                 L1_ACCESS_PERM_SYS_ONLY_READ_ONLY);
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
