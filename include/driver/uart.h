#ifndef _DRIVER_UART_H_

void init_uart();
void uart_putchar(unsigned char c);
unsigned char uart_getchar();

#define _DRIVER_UART_H_
#endif
