#ifndef CONFIG_H
#define CONFIG_H

// Undefine if using on real hardware
#define __QEMU__

#define UART_INPUT_BUFFER_SIZE 128
// timer interrupt interval in microseconds
#define LOCAL_TIMER_US 1000000

// clang-format off
// QEMU is a lot faster than the actual hardware, thus needing a higher value.
#ifdef __QEMU__
	#define BUSY_WAIT_COUNTER 3000000
#else
	#define BUSY_WAIT_COUNTER 30000
#endif /* __QEMU__ */
// clang-format on

#define LOGGING_ENABLED 1
// See `include/std/log.h`
#define LOGGING_LEVEL 99

// Number of processes and threads per process supported
#define N_PROCESSES 8
#define N_THREADS_PER_PROCESS 4
#define N_THREADS (N_PROCESSES * N_THREADS_PER_PROCESS)

#endif /* CONFIG_H */
