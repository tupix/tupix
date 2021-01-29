.section .init

_start:
	/* Erkennen ob Hypervisor Mode aktiv */
	mrs r0, cpsr
	and r0, r0, #0x1F
	mov r1, #0x1A	//Modebits = 0x1A = Hypervisor Mode
	cmp r0, r1		//Vergleichen und Statusbits setzen für bedingte Sprünge

	/* Wenn Hypemode erkannt: abschalten (sprung nach _exitHyper)
	 * wenn nicht erkannt: dann weitermachen (weiterlaufen nach _tupix)
	 */
	beq _exitHyper

	/* Qemu startet immer alle 4 Kerne
	 * Wir lassen alle anderen Kerne endless loopen
	 */
_loopCores:
	/* Id des Cpu Cores Abfragen */
	mrc p15, 0, r0, c0, c0, 5
	/* Falls nicht core 0 -> Endless loop */
	tst r0, #3
	bne .Lend

.extern _ivt
_tupix:
    bl _init_modes

    /* setzen, der ivt base-address im VBAR */
    ldr r0, =_ivt
    mcr p15, 0, r0, c12, c0, 0

	/* Zu c Code springen */
	bl start_kernel

.Lend:
	WFI // wait for interrupt
	b .Lend

_exitHyper:

	/* Rücksprungadresse ins Hypervisor Rücksprungregister schreiben.
	 * Dies ist das Label _tupix für die normale Programmausfürung.
	 */
	ldr lr, =_loopCores
	msr ELR_hyp, lr

	/* Der Hypervisor mode ist auch "nur" eine etwas kompliziertere Exception.
	 * Also springen wir aus dieser zurück.
	 * (Rücksprunge in Aufgabe 2 sind anders zu lösen!).
	 * Dazu zunächst das CPSR auslesen und die Modebits zum Supervisor ändern.
	 * Danach dies in das SPSR für den Rücksprung schreiben.
	 */

	mrs r0, cpsr
	bic r0, r0, #0x1F
	orr r0, r0, #0x13	//Supervisormode
	msr spsr_hyp, r0
	eret

_init_modes:
	// Enable IRQ and disable FIQ
	cpsid f
	cpsie i

    // supervisor sp
	ldr r0, =0x002FFFFF
    mov sp, r0
    // Initialize other stack pointers
    ldr r0, =0x002FFBFF
    msr sp_und, r0
    ldr r0, =0x002FF7FF
    msr sp_abt, r0
    ldr r0, =0x002FF3FF
    msr sp_irq, r0
    ldr r0, =0x002FEFFF
    msr sp_fiq, r0
    ldr r0, =0x002FEBFF
    msr sp_usr, r0

    mov pc, lr
