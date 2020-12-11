// Nächste Zeile auskommentieren falls auf der Hardware getestet wird
#define __QEMU__

// Wir testen nur mit Werten die durch 2^n darstellbar sind
#define UART_INPUT_BUFFER_SIZE 128
// Timer Interrupt Interval zum testen in Mikrosekunden
#define LOCAL_TIMER_US 1000000

// clang-format off
// QEMU is a lot faster than the actual hardware, thus needing a higher value.
#ifdef __QEMU__
	#define BUSY_WAIT_COUNTER 3000000
#else
	#define BUSY_WAIT_COUNTER 30000
#endif /* __QEMU__ */
// clang-format on

#define LOGGING_ENABLED

