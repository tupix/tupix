#include <driver/uart.h>

#include <config.h>

#include <driver/BCM2836.h>

#include <data/types.h>

#include <system/assert.h>

#include <std/bits.h>
#include <std/log.h>
#include <std/mem.h>
#include <std/util.h>

#define UART_BASE (0x7E201000 - MMU_BASE_OFFSET)

struct uart {
	uint32 dr;           // data register
	uint32 rsrecr;       //
	uint32 padding0[4];  //
	uint32 fr;           // flag register
	uint32 padding1;     //
	uint32 ilpr;         // unused
	uint32 ibrd;         //
	uint32 fbrd;         //
	uint32 lcrh;         // line control register
	uint32 cr;           // control register
	uint32 ifls;         // interrupt FIFO level select register
	uint32 imsc;         // interrupt mask set/clear register
	uint32 ris;          //
	uint32 mis;          //
	uint32 icr;          //
	uint32 dmacr;        //
	uint32 padding2[13]; //
	uint32 itcr;         //
	uint32 itip;         //
	uint32 itop;         //
	uint32 tdr;          //
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
	CR_RXE    = 9, // receive enable
	CR_TXE    = 8, // transmit enable
	CR_UARTEN = 0, // UART enable
};

enum imsc_bit_field {
	// clang-format off
	IMSC_OEIM = 10, // overrun error interrupt mask
	IMSC_BEIM = 9,  // break error interrupt mask
	IMSC_PEIM = 8,  // parity error interrupt mask
	IMSC_FEIM = 7,  // framing error interrupt mask
	IMSC_RTIM = 6,  // receive timeout mask
	IMSC_TXIM = 5,  // transmit interrupt mask
	IMSC_RXIM = 4,  // receive interrupt mask
	/* 2,3 unsupported, read as don't care */
	IMSC_CTSMIM = 1, // nUARTCTS modem interrupt mask
	/* 0 unsupported, read as don't care */
	// clang-format on
};

enum mis_bit_field {
	MIS_RXMIS = 4,
};

static volatile struct uart* const uart = (struct uart*)UART_BASE;
static struct uart_queue queue;

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

/*
 * Pop next buffered character from uart queue. This will block until at least
 * one character is available.
 */
char
uart_pop_char()
{
	char c = uart_peek_char();
	circle_forward(queue.head, queue.size);
	--(queue.count);
	return c;
}

/*
 * Get next buffered character from uart queue without popping it. This will
 * block until at least one character is available.
 */
char
uart_peek_char()
{
	return queue.chars[queue.head];
}

/*
 * Put received character into queue.
 */
bool
uart_push_char()
{
	if (IS_SET(uart->fr, FR_RXFE)) {
		log(WARNING, "UART Receive FIFO empty, cannot read character.");
		return false;
	}

	unsigned char c = (unsigned char)(uart->dr & 0xff);

	if (queue.count >= queue.size) {
		log(WARNING, "UART queue full. Discarding current character");
		return false;
	}
	queue.chars[queue.tail] = c;
	circle_forward(queue.tail, queue.size);
	++(queue.count);
	return true;
}

bool
uart_queue_is_emtpy()
{
	return (queue.count == 0);
}

bool
uart_is_interrupting()
{
	return IS_SET(uart->mis, MIS_RXMIS);
}

void
init_uart()
{
	// Initialize ringbuffer
	memset(&queue, 0, sizeof(queue));
	queue.size = sizeof(queue.chars) / sizeof(*(queue.chars));
	ASSERTM(queue.size, "UART Queue size needs a nonzero size");

	// Disable the UART.
	CLEAR_BIT(uart->cr, CR_UARTEN);
	// Flush the transmit FIFO
	CLEAR_BIT(uart->lcrh, LCRH_FEN);

	// Reprogram the UART
	SET_BIT(uart->cr, CR_RXE);
	SET_BIT(uart->cr, CR_TXE);

	SET_BIT_TO(uart->lcrh, LCRH_WLEN, LCRH_WLEN_8,
	           LCRH_WLEN_VAL_LEN); // set word length

	CLEAR_BIT(uart->lcrh, LCRH_FEN); // disable transmit and receive FIFO

	uart->imsc = 0;                 // clear all UART interrupt bits
	SET_BIT(uart->imsc, IMSC_RXIM); // enable receive interrupt

	// Enable the UART.
	SET_BIT(uart->cr, CR_UARTEN);
	log(LOG, "UART initialized.");
}

void
uart_put_char(unsigned char c)
{
	// Wait until transmit FIFO is not full
	while (IS_SET(uart->fr, FR_TXFF)) {}
	uart->dr = c;
}
