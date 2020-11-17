#include <driver/uart.h>

#include <driver/BCM2836.h>
#include <std/util.h>

#define UART_BASE (0x7E201000 - MMU_BASE_OFFSET)

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
	unsigned int ifls;		   // interrupt FIFO level select register
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

// Possible values for the LCRH word length
enum lcrh_wlen {
	LCRH_WLEN_8 = 0b11,
	LCRH_WLEN_7 = 0b10,
	LCRH_WLEN_6 = 0b01,
	LCRH_WLEN_5 = 0b00,
};

enum cr_bit_field {
	CR_RXE	  = 9, // receive enable
	CR_TXE	  = 8, // transmit enable
	CR_UARTEN = 0, // UART enable
};

enum ifls_bit_field {
	IFLS_RXIFLSEL = 3, // receive interrupt FIFO level select
	IFLS_TXIFLSEL = 2, // transmit interrupt FIFO level select
};

// Possible values for the IFLS_*IFLSEL fields (1_8 => 1/8)
enum ifls_iflsel {
	IFLS_IFLSEL_1_8 = 0b000,
	IFLS_IFLSEL_1_4 = 0b001,
	IFLS_IFLSEL_1_2 = 0b010,
	IFLS_IFLSEL_3_4 = 0b011,
	IFLS_IFLSEL_7_8 = 0b100,
};

enum imsc_bit_field {
	IMSC_OEIM	= 10, // overrun error interrupt mask
	IMSC_BEIM	= 9,  // break error interrupt mask
	IMSC_PEIM	= 8,  // parity error interrupt mask
	IMSC_FEIM	= 7,  // framing error interrupt mask
	IMSC_RTIM	= 6,  // receive timeout mask
	IMSC_TEIM	= 5,  // transmit interrupt mask
	IMSC_RXIM	= 4,  // receive interrupt mask
	IMSC_CTSMIM = 1,  // nUARTCTS modem interrupt mask
};

static volatile struct uart* const uart = (struct uart*)UART_BASE;

void init_uart()
{
	// Disable the UART.
	CLEAR_BIT(uart->cr, (uint32)CR_UARTEN);
	// Flush the transmit FIFO
	CLEAR_BIT(uart->lcrh, (uint32)LCRH_FEN);

	// Reprogram the UART
	SET_BIT(uart->cr, (uint32)CR_RXE);
	SET_BIT(uart->cr, (uint32)CR_TXE);

	SET_BIT(uart->lcrh, (uint32)LCRH_FEN); // enable transmit and receive FIFO
	uart->lcrh |= (LCRH_WLEN_8 << LCRH_WLEN); // set word length

	uart->imsc = 0; // clear all UART interrupt bits

	uart->ifls |= (IFLS_IFLSEL_1_8 << IFLS_RXIFLSEL); // rx interrupt trigger

	// Enable the UART.
	SET_BIT(uart->cr, (uint32)CR_UARTEN);
}

void uart_putchar(unsigned char c)
{
	// wait until transmit FIFO is not full
	while (IS_SET(uart->fr, FR_TXFF)) {}
	uart->dr = c;
}

unsigned char uart_getchar()
{
	// wait until receive FIFO is not empty
	while (IS_SET(uart->fr, FR_RXFE)) {}
	return (unsigned char)(uart->dr & 0xff);
}
