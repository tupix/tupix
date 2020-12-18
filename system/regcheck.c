#include <config.h>
#include <std/io.h> // Platzahlter für die Headerdatei, die euer kprintf enthält

void _check_return_jump(void);
void _check_registers(void);
void _check_cpsr(void);
void _check_spsr(void);

void
register_checker(void)
{
	unsigned int cpsr;
	asm("mrs %0, cpsr" : "=r"(cpsr));
	cpsr &= 0x1f;

	kprintf("\nBitte sicherstellen, dass waehrend der Tests Interrupts in ausreichender\n"
	        "Menge auftreten und auch durch euch behandelt werden!\n"
	        "\n"
	        "Die Dauer jedes Tests ist angegeben. Weicht sie stark ab, habt ihr\n"
	        "wahrscheinlich ein nicht durch die Tests abgedecktes Problem. (Zudem haengen\n"
	        "CPSR- und General-Purpose-Register-Test sehr voneinander ab: CPSR-Test braucht\n"
	        "Register, Register-Test brauch Flags. Also Registerdump kritisch betrachten.)\n"
	        "\n"
	        "Wenn eine Missstand festgestellt wird, wird eine Undef-Exception ausgelöst.\n"
	        "Mittels objdump, Quellen und Registersatz sollte sich das Problem einkreisen\n"
	        "lassen.\n"
	        "\n"
	        "(Alle Angaben ohne Gewaehr.)\n\n");

	if (cpsr == 0b10010) {
		kprintf(">>> Prozessor ist im IRQ-Modus => LR und SPSR verloren nach Interrupt\n"
		        ">>> (Insbesondere geht der Rücksprung aus dem ersten Test schief.)\n"
		        "\n"
		        ">>> Abbruch!\n");
		return;
	}

	kprintf("Pruefe richtige Ruecksprung-Adresse (ca. 5 Sekunden)\n");
	_check_return_jump();

	kprintf("Pruefe auf Register-Aenderungen (R0-R14, ca. 10 Sekunden)\n");
#ifdef __QEMU__
	for (unsigned int i = 0; i < 30; i++) {
#endif
		_check_registers();
#ifdef __QEMU__
	}
#endif

	if (cpsr == 0b10000) {
		kprintf("\n>>> Prozessor ist im User-Modus => Keine Pruefung auf CPSR-Aenderungen,\n"
		        ">>> da dort Modus-Bits geändert werden. (Wie wäre es mit dem System-Modus?)\n\n");
	} else {
		kprintf("Pruefe auf CPSR-Aenderungen (ca. 15 Sekunden)\n");
		_check_cpsr();
	}

	if (cpsr == 0b10000 || cpsr == 0b11111) {
		kprintf("\n>>> Prozessor ist im User- oder System-Modus => Keine Pruefung auf SPSR-Aenderungen,\n"
		        ">>> da es dieses Register nicht gibt. (Wie wäre es mit dem Supervisor-Modus?)\n\n");
	} else {
		kprintf("Pruefe auf SPSR-Aenderungen (ca. 15 Sekunden)\n");
		_check_spsr();
	}

	kprintf("Fertig! Es _scheint_ so, als wuerde es funktionieren.\n");
}
