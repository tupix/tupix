#ifndef SERIAL_H
#define SERIAL_H

unsigned int calc_digits(unsigned int n, unsigned int base);
char* ltostr(long n, unsigned int base, char* str);
char* ultostr(unsigned long n, unsigned int base, char* str);
void kputchar(unsigned char c);
void kputs(const char* s);
void kprintf(const char* format, ...);

#endif
