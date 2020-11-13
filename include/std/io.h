#ifndef SYSTEM_IO_H

void kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define SYSTEM_IO_H
#endif
