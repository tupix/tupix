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
    push { sp, pc, lr, r0 - r12 }
    mov r0, sp
    // TODO: load all other registers on to the stack
    b print_registers

#if 0
    // TODO: these here!
    fiq_pc, fiq_lr, fiq_sp,
    irq_pc, irq_lr, irq_sp,
    abt_pc, abt_lr, abt_sp,
    und_pc, und_lr, und_sp,
    svc_pc, svc_lr, svc_sp,
    usr_pc, usr_lr, usr_sp,
    spsr, cpsr,
#endif

.global _init_sp
_init_sp:
    // supervisor mode
    mov sp, #0x8000000

    ldr r0, =0x7FFFC00
    msr sp_und, r0
    ldr r0, =0x7FFF800
    msr sp_abt, r0
    ldr r0, =0x7FFF400
    msr sp_irq, r0
    ldr r0, =0x7FFF000
    msr sp_fiq, r0
    ldr r0, =0x7FFEC00
    msr sp_usr, r0

    mov pc, lr
