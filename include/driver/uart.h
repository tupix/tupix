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
char uart_pop_char();
char uart_peek_char();
bool uart_queue_is_emtpy();

bool uart_push_char();
bool uart_is_interrupting();

#endif /* DRIVER_UART_H */
