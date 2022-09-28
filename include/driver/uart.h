#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include <config.h>
#include <data/types.h>

struct uart_queue {
	size_t size, count;
	size_t head, tail;
	char chars[UART_INPUT_BUFFER_SIZE];
};

void init_uart();

void uart_put_char(unsigned char c);

/*
 * Pop next buffered character from uart queue. This will block until at least
 * one character is available.
 */
char uart_pop_char();

/*
 * Get next buffered character from uart queue without popping it. This will
 * block until at least one character is available.
 */
char uart_peek_char();

bool uart_queue_is_emtpy();

/*
 * Put received character into queue.
 */
bool uart_push_char();

bool uart_is_interrupting();

#endif /* DRIVER_UART_H */
