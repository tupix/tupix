#ifndef USER_SYSTEM_CALLS_H
#define USER_SYSTEM_CALLS_H

#include <data/types.h>

void exit();
char getchar();
void putchar(unsigned char c);
void wait(uint32 duration);
void create_thread(void (*func)(void*), const void* args, uint32 args_size);

#endif /* USER_SYSTEM_CALLS_H */
