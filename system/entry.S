.section .init

.global _start
_start:
	// Check if hypervisor mode is active
	mrs r0, cpsr
	and r0, r0, #0x1F
    // modebits = 0x1A = hypervisor mode
	mov r1, #0x1A
	cmp r0, r1

    // if in Hypervisor mode: shutdown
	beq _exitHyper

     /*
      * NOTE(Aurel): Qemu always starts all 4 cores. As we are currently only
      * using one we send all other cores to idle.
      */
_loopCores:
    // get core id
	mrc p15, 0, r0, c0, c0, 5
    // if not core 0: loop endlessly
	tst r0, #3
	bne .Lend

.extern _ivt
_tupix:
    bl _init_modes

    // set ivt base-address in VBAR
    ldr r0, =_ivt
    mcr p15, 0, r0, c12, c0, 0

	// jump to c code
	bl start_kernel

// endless loop
.Lend:
	WFI // wait for interrupt
	b .Lend

_exitHyper:

    // Write the return address into the hypervisor link-register.
	ldr lr, =_loopCores
	msr ELR_hyp, lr

	mrs r0, cpsr
	bic r0, r0, #0x1F
    // Supervisormode = 0x13
	orr r0, r0, #0x13
	msr spsr_hyp, r0
	eret

.extern _kstacks_start
_init_modes:
	// enable IRQ and disable FIQ
	cpsid f
	cpsie i

	// supervisor sp
	ldr r0, =_kstacks_start
	add r0, r0, #0x1000
	mov sp, r0
	// initialize other stack pointers
	// subtract 4KB (= 0x1000) from previous stack base
	add r0, r0, #0x1000
	msr sp_und, r0
	add r0, r0, #0x1000
	msr sp_abt, r0
	add r0, r0, #0x1000
	msr sp_irq, r0
	add r0, r0, #0x1000
	msr sp_fiq, r0
	add r0, r0, #0x1000
	msr sp_usr, r0

    mov pc, lr
