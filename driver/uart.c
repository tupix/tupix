#include <driver/uart.h>

#include <config.h>

#include <driver/BCM2836.h>

#include <std/types.h>
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

	LCRH_WLEN_VAL_LEN = 2, // The number of bits these values occupy
};

enum cr_bit_field {
	CR_RXE	  = 9, // receive enable
	CR_TXE	  = 8, // transmit enable
	CR_UARTEN = 0, // UART enable
};

enum imsc_bit_field {
	// clang-format off
	IMSC_OEIM = 10, // overrun error interrupt mask
	IMSC_BEIM = 9,	// break error interrupt mask
	IMSC_PEIM = 8,	// parity error interrupt mask
	IMSC_FEIM = 7,	// framing error interrupt mask
	IMSC_RTIM = 6,	// receive timeout mask
	IMSC_TXIM = 5,	// transmit interrupt mask
	IMSC_RXIM = 4,	// receive interrupt mask
	/* 2,3 unsupported, read as don't care */
	IMSC_CTSMIM = 1, // nUARTCTS modem interrupt mask
	/* 0 unsupported, read as don't care */
	// clang-format on
};

enum mis_bit_field {
	MIS_RXMIS = 4,
};

struct ringbuffer {
	uint32 size;
	uint32 tail, head;
	char buf[UART_INPUT_BUFFER_SIZE];
};

static volatile struct uart* const uart = (struct uart*)UART_BASE;
static struct ringbuffer* buffer;

char uart_getchar()
{
	while (buffer->head == buffer->tail) {}

	if (buffer->head == 0) {
		buffer->head = buffer->size;
	}
	--(buffer->head);
	return buffer->buf[buffer->head];
}

int uart_buffer_char()
{
	if (IS_SET(uart->fr, FR_RXFE))
		return -1;

	unsigned char c = (unsigned char)(uart->dr & 0xff);

	buffer->buf[buffer->head] = c;
	if (++(buffer->head) >= buffer->size) {
		buffer->head = 0;
	}
	if (buffer->head == buffer->tail) {
		if (++(buffer->tail) >= buffer->size) {
			buffer->tail = 0;
		}
	}
	return 0;
}

bool uart_is_interrupting()
{
	return IS_SET(uart->mis, MIS_RXMIS);
}

void init_uart()
{
	// initialize ringbuffer
	buffer->size = UART_INPUT_BUFFER_SIZE;
	buffer->tail = 0;
	buffer->head = 0;

	// Disable the UART.
	CLEAR_BIT(uart->cr, (uint32)CR_UARTEN);
	// Flush the transmit FIFO
	CLEAR_BIT(uart->lcrh, (uint32)LCRH_FEN);

	// Reprogram the UART
	SET_BIT(uart->cr, (uint32)CR_RXE);
	SET_BIT(uart->cr, (uint32)CR_TXE);

	SET_BIT_TO(uart->lcrh, (uint32)LCRH_WLEN, LCRH_WLEN_8,
			   (uint32)LCRH_WLEN_VAL_LEN); // set word length

	CLEAR_BIT(uart->lcrh,
			  (uint32)LCRH_FEN); // disable transmit and receive FIFO

	uart->imsc = 0;							// clear all UART interrupt bits
	SET_BIT(uart->imsc, (uint32)IMSC_RXIM); // enable receive interrupt

	// Enable the UART.
	SET_BIT(uart->cr, (uint32)CR_UARTEN);
}

// TODO(Aurel): Stop checking, if FIFOs are full or empty
void uart_putchar(unsigned char c)
{
	// wait until transmit FIFO is not full
	while (IS_SET(uart->fr, FR_TXFF)) {}
	uart->dr = c;
}
