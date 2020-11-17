#ifndef DRIVER_UART_H
#define DRIVER_UART_H

void init_uart();
void uart_putchar(unsigned char c);
unsigned char uart_getchar();

#endif /* DRIVER_UART_H */
