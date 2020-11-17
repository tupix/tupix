#ifndef _SYSTEM_IO_H_

void kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define _SYSTEM_IO_H_
#endif
