#ifndef ARCH_ARMV7_REGISTERS_H
#define ARCH_ARMV7_REGISTERS_H

#include <data/types.h>

/*
 * NOTE(Aurel): Do not change this, unless you know exactly what you are doing.
 * In that case, also change all the asm exception handlers.
 */
struct mode_registers {
	uint32 lr;
	uint32 sp;
	uint32 spsr;
};

/*
 * NOTE(Aurel): Do not change this, unless you know exactly what you are doing.
 * In that case, also change all the asm exception handlers.
 */
struct general_registers {
	uint32 r0;
	uint32 r1;
	uint32 r2;
	uint32 r3;
	uint32 r4;
	uint32 r5;
	uint32 r6;
	uint32 r7;
	uint32 r8;
	uint32 r9;
	uint32 r10;
	uint32 r11;
	uint32 r12;

	uint32 lr;
	uint32 pc;
	uint32 sp;
};

/*
 * NOTE(Aurel): Do not change this, unless you know exactly what you are doing.
 * In that case, also change all the asm exception handlers.
 */
struct registers {
	struct mode_registers fiq;
	struct mode_registers irq;
	struct mode_registers abt;
	struct mode_registers und;
	struct mode_registers svc;

	// TODO: Create mode_registers usr that always contains empty spsr
	uint32 usr_lr;
	uint32 usr_sp;

	uint32 cpsr;
	uint32 spsr;

	struct general_registers gr;
};

#endif /* ARCH_ARMV7_REGISTERS_H */
