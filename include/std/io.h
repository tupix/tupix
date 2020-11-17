#ifndef SYSTEM_IO_H
#define SYSTEM_IO_H

void kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif /* SYSTEM_IO_H */
