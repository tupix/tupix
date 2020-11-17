#include <driver/uart.h>

#include <std/util.h>

#define UART_BASE (0x7E201000 - 0x3F000000)

struct uart {
	unsigned int dr;		   // data register
	unsigned int rsrecr;	   //
	unsigned int padding0[4];  //
	unsigned int fr;		   // flag register
	unsigned int padding1;	   //
	unsigned int ilpr;		   // unused
	unsigned int ibrd;		   //
	unsigned int fbrd;		   //
	unsigned int lcrh;		   // line control register
	unsigned int cr;		   // control register
	unsigned int ifls;		   //
	unsigned int imsc;		   // interrupt mask set/clear register
	unsigned int ris;		   //
	unsigned int mis;		   //
	unsigned int icr;		   //
	unsigned int dmacr;		   //
	unsigned int padding2[13]; //
	unsigned int itcr;		   //
	unsigned int itip;		   //
	unsigned int itop;		   //
	unsigned int tdr;		   //
};

enum fr_bit_field {
	FR_TXFE = 7, // transmit FIFO empty
	FR_RXFF = 6, // receive FIFO full
	FR_TXFF = 5, // transmit FIFO full
	FR_RXFE = 4, // receive FIFO empty
	FR_BUSY = 3, // busy transmitting data
};

enum lcrh_bit_field {
	LCRH_WLEN = 5, // word length
	LCRH_FEN  = 4, // FIFO enable
};

enum cr_bit_field {
	CR_RXE	  = 9, // receive enable
	CR_TXE	  = 8, // transmit enable
	CR_UARTEN = 0, // UART enable
};

static volatile struct uart* const uart_i = (struct uart*)UART_BASE;

void init_uart()
{
	// Disable the UART.
	clear_bit(&(uart_i->cr), CR_UARTEN);
	// Flush the transmit FIFO
	clear_bit(&(uart_i->lcrh), LCRH_FEN);

	// Reprogram the UART
	set_bit(&(uart_i->cr), CR_RXE);
	set_bit(&(uart_i->cr), CR_TXE);

	set_bit(&(uart_i->lcrh), LCRH_FEN);	 // enable transmit and receive FIFO
	uart_i->lcrh |= (0b11 << LCRH_WLEN); // set word length to 8 bit

	uart_i->imsc = 0; // clear all UART interrupt bits

	// Enable the UART.
	set_bit(&(uart_i->cr), CR_UARTEN);
}

void uart_putchar(unsigned char c)
{
	// wait until transmit FIFO is not full
	while (is_set(uart_i->fr, FR_TXFF)) {}
	uart_i->dr = c;
}

unsigned char uart_getchar()
{
	// wait until receive FIFO is not empty
	while (is_set(uart_i->fr, FR_RXFE)) {}
	return (unsigned char)(uart_i->dr & 0xff);
}
