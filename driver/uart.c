#include <driver/uart.h>

#define UART_BASE (0x7E201000 - 0x3F000000)

enum fr_bit_field {
	FR_BUSY = 3, // busy transmitting data
	FR_RXFE = 4, // receive FIFO empty
	FR_TXFF = 5, // transmit FIFO full
	FR_RXFF = 6, // receive FIFO full
	FR_TXFE = 7	 // transmit FIFO empty
};

struct uart {
	unsigned int dr; // data register
	unsigned int rsrecr;
	unsigned int padding0[4];
	unsigned int fr; // flag register
	unsigned int padding1;
	unsigned int ilpr; // unused
	unsigned int ibrd;
	unsigned int fbrd;
	unsigned int lcrh;
	unsigned int cr; // control register
	unsigned int ifls;
	unsigned int imsc;
	unsigned int ris;
	unsigned int mis;
	unsigned int icr;
	unsigned int dmacr;
	unsigned int padding2[13];
	unsigned int itcr;
	unsigned int itip;
	unsigned int itop;
	unsigned int tdr;
};

static volatile struct uart* const uart_i = (struct uart*)UART_BASE;

void kputchar(unsigned char c)
{
	// TODO(aurel): Check if buffer is empty? What else needs to happen to make this safe?
	// Check flag register (FR) TXFF (bit 5) - Transmit FIFO full
	// Maybe also BUSY (bit 3) - busy transmitting data
	uart_i->dr = c;
}

unsigned char kgetchar()
{
	// TODO(aurel): Check if buffer has been written to? What else needs to happen to make this safe?
	// Check flag register (FR) RXFE (bit 4) - Receive FIFO empty
	char c = (unsigned char)(uart_i->dr & 0xff);
	return c;
}