#ifndef SERIAL_H
#define SERIAL_H

void kputchar(unsigned char c);

// TODO(aurel): Move somewhere else. stdio.c?
unsigned int calc_digits(unsigned int n, unsigned int base);
char* ltostr(long n, unsigned int base, char* str, unsigned int* len);
char* ultostr(unsigned long n, unsigned int base, char* str, unsigned int* len);
void kputchar(unsigned char c);
void kprint(const char* s);
void kprintf(const char* format, ...);
unsigned int str_len(const char* str);

#endif
