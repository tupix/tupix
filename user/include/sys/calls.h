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
 * Wait for the given duration in time-slides.
 * NOTE(Aurel): wait(0) is basically a noop with a mode-switch, just don't use
 * it.
 *
 * @param duration scheduler time-slides to wait until execution continues.
 */
void wait(uint32 duration);

/**
 * Create a new thread.
 *
 * @param func function the new thread executes.
 * @param args pointer to an array of arguments to pass to the function.
 * @param args_size size of `args`.
 */
void create_thread(void (*func)(void*), const void* args, uint32 args_size);

#endif /* USER_SYSTEM_CALLS_H */
