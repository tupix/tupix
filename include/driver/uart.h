#ifndef DRI_UART_H

void init_uart();
void kputchar(unsigned char c);
unsigned char kgetchar();

#define DRI_UART_H
#endif
