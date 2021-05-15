#ifndef USER_SYSTEM_CALLS_H
#define USER_SYSTEM_CALLS_H

#include <data/types.h>

/**
 * Kill your thread.
 */
void exit();

/**
 * Request a single char from the serial interface (UART).
 *
 * @return the char read from the serial interface.
 */
char getchar();

/**
 * Put a single char onto the serial interface (UART).
 *
 * @param c character to print into the serial interface.
 */
void putchar(unsigned char c);

/**
 * Wait for at-least the given duration in time-slides. This might take longer
 * than you expect.
 * NOTE(Aurel): This should not be used in actual time-sensitive code.
 * NOTE(Aurel): wait(0) is basically a noop with a mode-switch, just don't use
 *				it.
 *
 * @param duration scheduler time-slides to wait until execution continues.
 */
void wait(size_t duration);

/**
 * Create a new process with it's own memory space.
 *
 * @param func function the new thread executes.
 * @param args pointer a block of memory that is passed and can be used for
 *             arguments for `func`.
 * @param args_size size of `args`.
 */
void fork(void (*func)(void*), const void* args, size_t args_size);

/**
 * Create a new thread with the same memory space as the creating thread.
 *
 * @param func function the new thread executes.
 * @param args pointer to an array of arguments to pass to the function.
 * @param args_size size of `args`.
 */
void create_thread(void (*func)(void*), const void* args, size_t args_size);

#endif /* USER_SYSTEM_CALLS_H */
