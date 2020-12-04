#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include <std/types.h>

void init_uart();
void uart_putchar(unsigned char c);
char uart_getchar();

int uart_buffer_char();
bool uart_is_interrupting();

#endif /* DRIVER_UART_H */
