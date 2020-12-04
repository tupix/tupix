#ifndef DRIVER_UART_H
#define DRIVER_UART_H

void init_uart();
void uart_putchar(unsigned char c);
int uart_getchar(char* c);

int uart_buffer_char();

#endif /* DRIVER_UART_H */
