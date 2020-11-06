#define SERIAL_BASE (0x7E201000 - 0x3F000000)

struct serial {
    unsigned int dr; // data register
    unsigned int rsrecr;
    unsigned int fr; // flag register
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
    unsigned int itcr;
    unsigned int itip;
    unsigned int itop;
    unsigned int tdr;
}

static volatile struct serial* const serial_i = (struct serial*)SERIAL_BASE;
