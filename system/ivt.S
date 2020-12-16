.global _ivt
.balign 32
_ivt:
	b   reset
	ldr pc, _undefined_instruction
	ldr pc, _software_interrupt
	ldr pc, _prefetch_abort
	ldr pc, _data_abort
	ldr pc, _not_used
	ldr pc, _irq
	ldr pc, _fiq

	_undefined_instruction: .word undefined_instruction
	_software_interrupt:    .word software_interrupt
	_prefetch_abort:        .word prefetch_abort
	_data_abort:            .word data_abort
	_not_used:              .word not_used
	_irq:                   .word irq
	_fiq:                   .word fiq

fiq: not_used:
reset:
	b _start

undefined_instruction:
	/*
	 * NOTE(Aurel): The registers are positioned in here as they are in the
	 * struct registers in isr.c. They however have to be pushed a little
	 * differently hence why they are loaded weirdly into the processor
	 * registers a little.
	 */
	push { sp }
	push { pc }
	push { lr }
	push { r0 - r12 }

	mrs r1, spsr
	mrs r0, cpsr
	push {r0 - r1}

	// usr
	mrs r8, lr_usr
	mrs r9, sp_usr
	// svc
	mrs r5, lr_svc
	mrs r6, sp_svc
	mrs r7, spsr_svc
	// und
	mov r2, lr
	mov r3, sp
	mrs r4, spsr
	push {r2 - r9}

	// abt
	mrs r6, lr_abt
	mrs r7, sp_abt
	mrs r8, spsr_abt
	// irq
	mrs r3, lr_irq
	mrs r4, sp_irq
	mrs r5, spsr_irq
	// fiq
	mrs r0, lr_fiq
	mrs r1, sp_fiq
	mrs r2, spsr_fiq
	push {r0 - r8}

	/*
	 * NOTE(Aurel): Give current stack pointer as first argument. It points
	 * exactly to the end of where all the registers have been pushed to and is
	 * therefor the location at which the struct registers should be placed.
	 */
	mov r0, sp
	b undefined_instruction_handler

software_interrupt:
	/*
	 * NOTE(Aurel): The registers are positioned in here as they are in the
	 * struct registers in isr.c. They however have to be pushed a little
	 * differently hence why they are loaded weirdly into the processor
	 * registers a little.
	 */
	push { sp }
	push { pc }
	//sub lr, #4 // TODO(Aurel): In preperation for later.
	push { lr }
	push { r0 - r12 }

	mrs r1, spsr
	mrs r0, cpsr
	push {r0 - r1}

	// usr
	mrs r8, lr_usr
	mrs r9, sp_usr
	// svc
	mov r5, lr
	mov r6, sp
	mrs r7, spsr
	// und
	mrs r2, lr_und
	mrs r3, sp_und
	mrs r4, spsr_und
	push {r2 - r9}

	// abt
	mrs r6, lr_abt
	mrs r7, sp_abt
	mrs r8, spsr_abt
	// irq
	mrs r3, lr_irq
	mrs r4, sp_irq
	mrs r5, spsr_irq
	// fiq
	mrs r0, lr_fiq
	mrs r1, sp_fiq
	mrs r2, spsr_fiq
	push {r0 - r8}

	/*
	 * NOTE(Aurel): Give current stack pointer as first argument. It points
	 * exactly to the end of where all the registers have been pushed to and is
	 * therefor the location at which the struct registers should be placed.
	 */
	mov r0, sp
	/*
	 * NOTE(Aurel): Save the lr before branching and linking the c-function and
	 * pop it directly into the pc to return to the program
	 */
	push {lr}
	bl software_interrupt_handler
	pop {pc}

prefetch_abort:
	/*
	 * NOTE(Aurel): The registers are positioned in here as they are in the
	 * struct registers in isr.c. They however have to be pushed a little
	 * differently hence why they are loaded weirdly into the processor
	 * registers a little.
	 */
	push { sp }
	push { pc }
	push { lr }
	push { r0 - r12 }

	mrs r1, spsr
	mrs r0, cpsr
	push {r0 - r1}

	// usr
	mrs r8, lr_usr
	mrs r9, sp_usr
	// svc
	mrs r5, lr_svc
	mrs r6, sp_svc
	mrs r7, spsr_svc
	// und
	mrs r2, lr_und
	mrs r3, sp_und
	mrs r4, spsr_und
	push {r2 - r9}

	// abt
	mov r6, lr
	mov r7, sp
	mrs r8, spsr
	// irq
	mrs r3, lr_irq
	mrs r4, sp_irq
	mrs r5, spsr_irq
	// fiq
	mrs r0, lr_fiq
	mrs r1, sp_fiq
	mrs r2, spsr_fiq
	push {r0 - r8}

	/*
	 * NOTE(Aurel): Give current stack pointer as first argument. It points
	 * exactly to the end of where all the registers have been pushed to and is
	 * therefor the location at which the struct registers should be placed.
	 */
	mov r0, sp
	b prefetch_abort_handler

data_abort:
	/*
	 * NOTE(Aurel): The registers are positioned in here as they are in the
	 * struct registers in isr.c. They however have to be pushed a little
	 * differently hence why they are loaded weirdly into the processor
	 * registers a little.
	 */
	push { sp }
	push { pc }
	push { lr }
	push { r0 - r12 }

	mrs r1, spsr
	mrs r0, cpsr
	push {r0 - r1}

	// usr
	mrs r8, lr_usr
	mrs r9, sp_usr
	// svc
	mrs r5, lr_svc
	mrs r6, sp_svc
	mrs r7, spsr_svc
	// und
	mrs r2, lr_und
	mrs r3, sp_und
	mrs r4, spsr_und
	push {r2 - r9}

	// abt
	mov r6, lr
	mov r7, sp
	mrs r8, spsr
	// irq
	mrs r3, lr_irq
	mrs r4, sp_irq
	mrs r5, spsr_irq
	// fiq
	mrs r0, lr_fiq
	mrs r1, sp_fiq
	mrs r2, spsr_fiq
	push {r0 - r8}

	/*
	 * NOTE(Aurel): Give current stack pointer as first argument. It points
	 * exactly to the end of where all the registers have been pushed to and is
	 * therefor the location at which the struct registers should be placed.
	 */
	mov r0, sp
	b data_abort_handler

irq:
	/*
	 * NOTE(Aurel): The registers are positioned in here as they are in the
	 * struct registers in isr.c. They however have to be pushed a little
	 * differently hence why they are loaded weirdly into the processor
	 * registers a little.
	 */
	push { sp }
	push { pc }
	// Jump back to previous instruction on return
	sub lr, #4
	push { lr }
	push { r0 - r12 }

	mrs r1, spsr
	mrs r0, cpsr
	push {r0 - r1}

	// usr
	mrs r8, lr_usr
	mrs r9, sp_usr
	// svc
	mrs r5, lr_svc
	mrs r6, sp_svc
	mrs r7, spsr_svc
	// und
	mrs r2, lr_und
	mrs r3, sp_und
	mrs r4, spsr_und
	push {r2 - r9}

	// abt
	mrs r6, lr_abt
	mrs r7, sp_abt
	mrs r8, spsr_abt
	// irq
	mov r3, lr
	mov r4, sp
	mrs r5, spsr
	// fiq
	mrs r0, lr_fiq
	mrs r1, sp_fiq
	mrs r2, spsr_fiq
	push {r0 - r8}

	/*
	 * NOTE(Aurel): Give current stack pointer as first argument. It points
	 * exactly to the end of where all the registers have been pushed to and is
	 * therefor the location at which the struct registers should be placed.
	 */
	mov r0, sp
	bl irq_handler

	b exc_cleanup_return

exc_cleanup_return:
	/*
	 * NOTE(Aurel): Throw away the 19 pushed mode-specific register
	 * 19 * 4 = 76
	 */
	// Throw away all mode-specific register except for usr
	// #{fiq, irq, abt, und, svc} * 3 * 4 = 15 * 4 = 60
	add sp, #60

	// Restore usr register
	pop {r0}
	msr lr_usr, r0
	pop {r1}
	msr sp_usr, r1

	// Throw away cpsr and spsr
	// TODO: Keep?
	add sp, #8

	// Restore the old register values
	pop {r0 - r12}
	pop {lr}
	// Throw away pc and sp (2 * 4 = 8)
	add sp, #8
	// Return and change mode
	movs pc, lr
