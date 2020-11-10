#include <driver/serial.h>

#define SERIAL_BASE (0x7E201000 - 0x3F000000)

struct serial {
	unsigned int dr; // data register
	unsigned int rsrecr;
	unsigned int padding0[4];
	unsigned int fr; // flag register
	unsigned int padding1;
	unsigned int ilpr;
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

static volatile struct serial* const serial_i = (struct serial*)SERIAL_BASE;

void kputchar(unsigned char c)
{
        // TODO(aurel): Check if buffer is empty? What else needs to happen to make this safe?
        serial_i->dr = c;
}

unsigned char kgetchar()
{
    // TODO(aurel): Check if buffer has been written to? What else needs to happen to make this safe?
    return (unsigned char)(serial_i->dr & 0xff);
}



