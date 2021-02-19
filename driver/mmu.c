#include <driver/mmu.h>

#include <system/assert.h>

#include <std/bits.h>
#include <std/log.h>

#include <std/mem.h>

extern uint32 _l1_start[N_L1_ENTRIES];
#define L1 _l1_start

extern uint32 _udata_begin[];
#define UDATA_USTACK_PHYS_MB ((uint32)_udata_begin >> 20)

__attribute__((aligned(1024)))
uint32 l1_udata_initialization_l2_table[N_L2_ENTRIES];

enum page_access_permission {
	PAGE_ACCESS_PERM_SYS_USER_FULL           = 0b011,
	PAGE_ACCESS_PERM_SYS_FULL_USER_READ_ONLY = 0b010,
	PAGE_ACCESS_PERM_SYS_USER_READ_ONLY      = 0b111,
	PAGE_ACCESS_PERM_SYS_ONLY_READ_ONLY      = 0b101,
	PAGE_ACCESS_PERM_SYS_ONLY_FULL           = 0b001,
	PAGE_ACCESS_PERM_NO                      = 0b000,
};

enum page_entry_type {
	PAGE_ENTRY_TYPE_FAULT = 0b00,

	L1_ENTRY_TYPE_L2_POINTER = 0b01,
	L1_ENTRY_TYPE_1MB_PAGE   = 0b10,

	L2_ENTRY_TYPE_LARGE_PAGE = 0b01,
	L2_ENTRY_TYPE_SMALL_PAGE = 0b10,

	PAGE_ENTRY_TYPE_SIZE = 2,
};

uint32
set_page_entry_type(uint32 entry, enum page_entry_type entry_type)
{
	/*
	 * NOTE(Aurel): We need to check for the type, as we don't want to overwrite
	 * bit 0 when we are dealing with a 1MB page L1-entry or small page l2-entry
	 * as that bit actually represents, whether privileged execution is not
	 * allowed. This way this function never overwrites any bits set
	 * beforehand.
	 */
	if (entry_type == L1_ENTRY_TYPE_1MB_PAGE ||
	    entry_type == L2_ENTRY_TYPE_SMALL_PAGE)
		SET_BIT(entry, 1);
	else
		SET_BIT_TO(entry, 0, entry_type, PAGE_ENTRY_TYPE_SIZE);

	return entry;
}

/*********** L1-TABLE HELPER ***********/
enum l1_1MB_page_entry_bit_field {
	L1_1MB_PAGE_BASE_ADDRESS = 20, // Physical address to map to
	L1_1MB_PAGE_NS           = 19, //
	L1_1MB_PAGE_0            = 18, //
	L1_1MB_PAGE_nG           = 17, // not Global -> process specific
	L1_1MB_PAGE_S            = 16, //
	L1_1MB_PAGE_AP_2         = 15, // Access Permission[2]
	L1_1MB_PAGE_TEX          = 12, //
	L1_1MB_PAGE_AP_0         = 10, // Access Permission[1:0]
	L1_1MB_PAGE_IMPL         = 9,  //
	L1_1MB_PAGE_DOMAIN       = 5,  //
	L1_1MB_PAGE_XN           = 4,  // eXecute Never
	L1_1MB_PAGE_C            = 3,  //
	L1_1MB_PAGE_B            = 2,  //
	L1_1MB_PAGE_1            = 1,  // must be 1
	L1_1MB_PAGE_PXN          = 0,  // Privileged eXecute Never
};

enum l1_l2_pointer_entry_bit_field {
	L1_L2_PAGE_BASE_ADDRESS = 10, // physical address to map to
	L1_L2_PAGE_IMPL         = 9,  // IMPLementation defined
	L1_L2_PAGE_DOMAIN       = 5,  //
	L1_L2_PAGE_SBZ          = 4,  // Should Be Zero
	L1_L2_PAGE_NS           = 3,  // Non-Secure
	L1_L2_PAGE_PXN          = 2,  // Privileged eXecute Never
	L1_L2_PAGE_0            = 1,  // must be 0
	L1_L2_PAGE_1            = 0,  // must be 1
};

uint32
set_l1_access_permission(uint32 entry, enum page_access_permission permission)
{
	/*
	 * NOTE(Aurel): Permission bits are split into two:
	 * L1_AP_0 takes the first two bits
	 * L1_AP_2 takes the third bit
	 */
	SET_BIT_TO(entry, L1_1MB_PAGE_AP_0, permission, 2);
	SET_BIT_TO(entry, L1_1MB_PAGE_AP_2, (permission >> 2), 1);
	return entry;
}

uint32
set_l1_base_address_of_index(uint32 entry, uint32 index)
{
	// clear base address bits and set them to the index
	entry &= 0x000fffff;
	entry |= (index << 20);
	return entry;
}

uint32
build_l1_1MB_page_entry(uint32 index, enum page_access_permission permission,
                        bool allow_execute, bool allow_privileged_execute)
{
	uint32 entry = 0;

	entry = set_page_entry_type(entry, L1_ENTRY_TYPE_1MB_PAGE);
	entry = set_l1_base_address_of_index(entry, index);
	entry = set_l1_access_permission(entry, permission);

	if (!allow_execute)
		SET_BIT(entry, L1_1MB_PAGE_XN);
	if (!allow_privileged_execute)
		SET_BIT(entry, L1_1MB_PAGE_PXN);

	return entry;
}

uint32
build_l1_l2_pointer_entry(uint32* l2_table, bool allow_privileged_execute)
{
	uint32 entry = (uint32)l2_table;

	entry = set_page_entry_type(entry, L1_ENTRY_TYPE_L2_POINTER);

	if (!allow_privileged_execute)
		SET_BIT(entry, L1_L2_PAGE_PXN);

	return entry;
}

/*********** \L1-TABLE HELPER ***********/

/*********** L2-TABLE HELPER ***********/
enum l2_small_page_entry {
	L2_SMALL_PAGE_BASE_ADDRESS = 12,
	L2_SMALL_PAGE_nG           = 11,
	L2_SMALL_PAGE_AP_2         = 10,
	L2_SMALL_PAGE_S            = 9,
	L2_SMALL_PAGE_TEX          = 6,
	L2_SMALL_PAGE_AP_0         = 4,
	L2_SMALL_PAGE_C            = 3,
	L2_SMALL_PAGE_B            = 2,
	L2_SMALL_PAGE_1            = 1,
	L2_SMALL_PAGE_XN           = 0,
};

uint32
set_l2_access_permission(uint32 entry, enum page_access_permission permission)
{
	/*
	 * NOTE(Aurel): Permission bits are split into two:
	 * L1_AP_0 takes the first two bits
	 * L1_AP_2 takes the third bit
	 */
	SET_BIT_TO(entry, L2_SMALL_PAGE_AP_0, permission, 2);
	SET_BIT_TO(entry, L2_SMALL_PAGE_AP_2, (permission >> 2), 1);
	return entry;
}

/**
 * TODO(Aurel): Think about the mb argument.
 * @arg mb - megabyte in physical address space.
 */
uint32
set_l2_base_address_of_index(uint32 mb, uint32 entry, uint32 index)
{
	// clear base address bits and set them to the index
	entry &= 0x00000fff;
	entry |= (index << 12);
	entry |= (mb << 20);
	return entry;
}

/**
 * @arg index into the l2 address. index \in [0..256]
 */
uint32
build_l2_4KB_page_entry(uint32 mb, uint32 index,
                        enum page_access_permission permission,
                        bool allow_execute)
{
	uint32 entry = 0;

	entry = set_page_entry_type(entry, L2_ENTRY_TYPE_SMALL_PAGE);
	entry = set_l2_base_address_of_index(mb, entry, index);
	entry = set_l2_access_permission(entry, permission);

	// NOTE(Aurel): Privileged execution is set in L1 pointer entry.
	if (!allow_execute)
		SET_BIT(entry, L2_SMALL_PAGE_XN);

	return entry;
}
/*********** \L2-TABLE HELPER ***********/

/*********** DACR HELPER ***********/
enum dacr_domain_access {
	// any access generates domain fault:
	DACR_DOMAIN_NO_ACCESS = 0b00,
	// access checked against access permission bits:
	DACR_DOMAIN_CLIENT_ACCESS = 0b01,
	// access not checked against access permission bits:
	DACR_DOMAIN_MANAGER_ACCESS = 0b11,

	DACR_DOMAIN_ACCESS_SIZE = 2,
};

uint32
set_dacr_domain_access(uint32 dacr, uint32 domain,
                       enum dacr_domain_access access)
{
	SET_BIT_TO(dacr, domain * 2, access, DACR_DOMAIN_ACCESS_SIZE);
	return dacr;
}
/*********** \DACR HELPER ***********/

void
init_l1()
{
	// Null all entries.
	memset(L1, 0, 0x4000);

	/*
	 * NOTE(Aurel): These indices correspond to the 6th hex-digit seen in
	 * kernel.lds or the base addresses of the hardware-components driver.
	 */
	// init code
	L1[0] = build_l1_1MB_page_entry(0, PAGE_ACCESS_PERM_SYS_ONLY_READ_ONLY,
	                                true, true);
	// kernel code
	L1[1] = build_l1_1MB_page_entry(1, PAGE_ACCESS_PERM_SYS_ONLY_READ_ONLY,
	                                true, true);
	// kernel data including stacks
	L1[2] = build_l1_1MB_page_entry(2, PAGE_ACCESS_PERM_SYS_ONLY_FULL, false,
	                                false);
	L1[3] = build_l1_1MB_page_entry(3, PAGE_ACCESS_PERM_SYS_ONLY_FULL, false,
	                                false);
	// user code
	L1[4] = build_l1_1MB_page_entry(4, PAGE_ACCESS_PERM_SYS_USER_READ_ONLY,
	                                true, false);
	// user data and stacks
	/*
	 * NOTE(Aurel): The user stacks share the same virtual address.
	 * NOTE(Aurel): As long as there is no thread there can't be a user stack
	 * hence why at initialization there is only one entry into the l2 table
	 * pointed to here enabling access to the user data and bss segments.
	 */
	l1_udata_initialization_l2_table[0] = build_l2_4KB_page_entry(
			UDATA_USTACK_PHYS_MB, 0, PAGE_ACCESS_PERM_SYS_USER_FULL, false);
	L1[UDATA_USTACK_PHYS_MB] =
			build_l1_l2_pointer_entry(l1_udata_initialization_l2_table, false);

	// hardware
	// Interrupt Controller
	L1[0x3F0] = build_l1_1MB_page_entry(0x3F0, PAGE_ACCESS_PERM_SYS_ONLY_FULL,
	                                    false, false);
	// UART
	L1[0x3F2] = build_l1_1MB_page_entry(0x3F2, PAGE_ACCESS_PERM_SYS_ONLY_FULL,
	                                    false, false);
	// TIMER
	L1[0x400] = build_l1_1MB_page_entry(0x400, PAGE_ACCESS_PERM_SYS_ONLY_FULL,
	                                    false, false);
	klog(LOG, "L1-table initialized.");
}

uint32
get_dacr_init_val(uint32 dacr)
{
	dacr = set_dacr_domain_access(dacr, 0, DACR_DOMAIN_CLIENT_ACCESS);
	return dacr;
}

uint32
get_ttbcr_init_val(uint32 ttbcr)
{
	// NOTE(Aurel): Setting the TTBCR to 0 means that it will only try to use
	// the TTBR0 mappings. That is exactly what we want.
	ttbcr = 0;
	return ttbcr;
}

void
init_process_memory(uint32 index, uint32* l2_table)
{
	klog(LOG, "Initializing process memory...");
	// TODO(Aurel): Should this really be an mmu-function?
	// as there is no thread in the process which is the case at initialization.
	memset(l2_table, 0, N_L2_ENTRIES * sizeof(*l2_table));
	l2_table[0] = build_l2_4KB_page_entry(
			/* mb  = */ index + UDATA_USTACK_PHYS_MB, 0,
			PAGE_ACCESS_PERM_SYS_USER_FULL, false);
	klog(LOG, "Done initializing process memory.");
}

void
init_thread_memory(size_t pid, size_t thread_index, uint32* l2_table)
{
	klog(LOG, "Initializing thread memory...");
#if 0
	klog(DEBUG,
	     "Initializing thread memory for process %i, thread %i, in l2_table at %p...",
	     pid, thread_index, l2_table);
#endif
	/*
	 * NOTE(Aurel): Stacks are located from the 6th MB upward, with the 0th stack exactly at
	 * 6MB in memory. Hence index + 6.
	 * The second entry is reserved for the thread stack. It should be read- and
	 * writable, but not executable.
	 */
	uint32 l2_entry = build_l2_4KB_page_entry(
			/* mb  = */ pid + 5, thread_index + 1,
			PAGE_ACCESS_PERM_SYS_USER_FULL, false);
	l2_table[thread_index * 2 + 1 + 1] = l2_entry;
	klog(LOG, "Done initializing thread memory.");
}

void
switch_memory(uint32* l2_table)
{
	// invalidate entire TLB
	asm("mcr p15, 0, r0, c8, c7, 0");

	// set base address to new l2_table
	// TODO(Aurel): Cleanup into function?
	L1[UDATA_USTACK_PHYS_MB] &= 0x000000ff;
	L1[UDATA_USTACK_PHYS_MB] |= (uint32)l2_table & 0xffffff00;
}
