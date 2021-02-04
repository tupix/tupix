.global init_mmu

.extern get_dacr_init_val
.extern get_ttbcr_init_val
.extern init_l1
.extern _l1_start
init_mmu:
	push { lr }

	// init dacr
	mrc p15, 0, r0, c2, c0, 0
	bl get_dacr_init_val
	mcr p15, 0, r0, c2, c0, 0

	// init ttbcr
	mrc p15, 0, r0, c2, c0, 2
	bl get_ttbcr_init_val
	mcr p15, 0, r0, c2, c0, 2

	// init L1-table and tell MMU where to find it
	bl init_l1
	ldr r0, =_l1_start
	mcr p15, 0, r0, c2, c0, 0

	// enable the MMU
	mrc p15, 0, r0, c1, c0, 0 // read SCTLR
	orr r0, r0, #0b1		  // SCTRL bit 0 - enable MMU
	mrc p15, 0, r0, c1, c0, 0 // write SCTLR

	pop { lr }
	movs pc, lr
