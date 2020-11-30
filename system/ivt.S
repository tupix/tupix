.global _ivt
.balign 64
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

// TODO: subtract return-offset.
// is the return offset 8 for undefined instruction?
undefined_instruction:
    // NOTE(Aurel): The registers are positioned in here as they are in the
    // struct. They however have to be pushed a little differently hence why
    // they are loaded weirdly into the processor registors a little.
    push { sp }
    push { pc }
    push { lr }
    push { r0 - r12 }

    mrs r1, spsr
    mrs r0, cpsr
    push {r0 - r1}

    // TODO(AUREL): Something is not quite right with the spsr of the modes.

    // usr
    mrs r5, lr_usr
    mrs r6, sp_usr
    // svc
    mrs r2, lr_svc
    mrs r3, sp_svc
    mrs r4, spsr_svc
    push {r2 - r6}

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
    b print_registers
