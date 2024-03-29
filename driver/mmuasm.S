.global init_mmu

.extern kprintf

// NOTE(Aurel): All these functions can be found in driver/mmu.c
.extern get_dacr_init_val
.extern get_ttbcr_init_val
.extern init_l1

/*
 * NOTE(Aurel): A symbol from the linker. See kernel.lds. This is where the
 * L1-table is being placed.
 */
.extern _l1_start
init_mmu:
	push { lr }

	/*
	 * NOTE(Aurel): Init DACR in a C-function which expects the current DACR to
	 * be given as the first argument. This way it can properly read, change,
	 * write.
	 */
	mrc p15, 0, r0, c3, c0, 0
	bl get_dacr_init_val
	mcr p15, 0, r0, c3, c0, 0

	// Init TTBCR the same way as the DACR.
	mrc p15, 0, r0, c2, c0, 2
	bl get_ttbcr_init_val
	mcr p15, 0, r0, c2, c0, 2

	// init L1-table and tell MMU where to find it
	bl init_l1
	ldr r0, =_l1_start
	mcr p15, 0, r0, c2, c0, 0 // write TTBR0

	// enable the MMU
	mrc p15, 0, r0, c1, c0, 0 // read SCTLR
	orr r0, r0, #0b1		  // SCTRL bit 0 - enable MMU
	mcr p15, 0, r0, c1, c0, 0 // write SCTLR

	pop { lr }
	movs pc, lr

.section .data
init_mmu_initialized_msg:
	.asciz "MMU initialized.\n\0"
