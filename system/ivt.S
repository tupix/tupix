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

not_used:
reset:
	b _start

undefined_instruction:
	// NOTE(Aurel): The registers are positioned in here as they are in the
	// struct. They however have to be pushed a little differently hence why
	// they are loaded weirdly into the processor registers a little.
	push { sp }
	push { pc }
	// TODO(Aurel): What is the offset? See documentation.
	// Maybe add 4 as we don't want to redo the undefined instruction
	//sub lr, #8
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

	// NOTE(Aurel): Give current stack pointer as first argument. It points
	// exactly to the end of where all the registers have been pushed to and is
	// therefor the location at which the struct registers should be placed.
	mov r0, sp
	b undefined_instruction_handler

// TODO: subtract return-offset.
software_interrupt:
	// NOTE(Aurel): The registers are positioned in here as they are in the
	// struct. They however have to be pushed a little differently hence why
	// they are loaded weirdly into the processor registers a little.
	push { sp }
	push { pc }
	sub lr, #8 // TODO(Aurel): What is the offset? See documentation.
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

	// NOTE(Aurel): Give current stack pointer as first argument. It points
	// exactly to the end of where all the registers have been pushed to and is
	// therefor the location at which the struct registers should be placed.
	// NOTE(Aurel): Save the lr before branching and linking the c-function and
	// pop it directly into the sp to return to the program
	mov r0, sp
	push {lr}
	bl software_interrupt_handler
	pop {pc}

prefetch_abort:
	// NOTE(Aurel): The registers are positioned in here as they are in the
	// struct. They however have to be pushed a little differently hence why
	// they are loaded weirdly into the processor registers a little.
	push { sp }
	push { pc }
	//sub lr, #8 // TODO(Aurel): What is the offset? See documentation.
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

	// NOTE(Aurel): Give current stack pointer as first argument. It points
	// exactly to the end of where all the registers have been pushed to and is
	// therefor the location at which the struct registers should be placed.
	mov r0, sp
	b prefetch_abort_handler

data_abort:
	// NOTE(Aurel): The registers are positioned in here as they are in the
	// struct. They however have to be pushed a little differently hence why
	// they are loaded weirdly into the processor registers a little.
	push { sp }
	push { pc }
	//sub lr, #8 // TODO(Aurel): What is the offset? See documentation.
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

	// NOTE(Aurel): Give current stack pointer as first argument. It points
	// exactly to the end of where all the registers have been pushed to and is
	// therefor the location at which the struct registers should be placed.
	mov r0, sp
	b data_abort_handler

irq:
	// NOTE(Aurel): The registers are positioned in here as they are in the
	// struct. They however have to be pushed a little differently hence why
	// they are loaded weirdly into the processor registers a little.
	push { sp }
	push { pc }
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

	// NOTE(Aurel): Give current stack pointer as first argument. It points
	// exactly to the end of where all the registers have been pushed to and is
	// therefor the location at which the struct registers should be placed.
	// NOTE(Aurel): Save the lr before branching and linking the c-function and
	// pop it directly into the sp to return to the program
	mov r0, sp
	push {lr}
	bl irq_handler
	pop {lr}
	b exc_cleanup_return

// TODO: subtract return-offset.
fiq:
	// NOTE(Aurel): The registers are positioned in here as they are in the
	// struct. They however have to be pushed a little differently hence why
	// they are loaded weirdly into the processor registers a little.
	push { sp }
	push { pc }
	sub lr, #8 // TODO(Aurel): What is the offset? See documentation.
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
	mrs r3, lr_irq
	mrs r4, sp_irq
	mrs r5, spsr_irq
	// fiq
	mov r0, lr
	mov r1, sp
	mrs r2, spsr
	push {r0 - r8}

	// NOTE(Aurel): Give current stack pointer as first argument. It points
	// exactly to the end of where all the registers have been pushed to and is
	// therefor the location at which the struct registers should be placed.
	// NOTE(Aurel): Save the lr before branching and linking the c-function and
	// pop it directly into the sp to return to the program
	mov r0, sp
	push {lr}
	bl fiq_handler
	pop {lr}
	b exc_cleanup_return


exc_cleanup_return:
    mrs r0, spsr
    msr cpsr, r0

    // TODO(Aurel): Maybe just increment to sp. Definitely needs a comment
    pop {r0 - r9}
    pop {r0 - r8}
    pop {r0 - r12}

    // jump back
    mov pc, lr
