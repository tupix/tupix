#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

void kill_me();
char getchar();
void putchar(unsigned char c);
void wait(size_t duration);
void create_thread(void (*func)(void*), const void* args, size_t args_size);

#endif /* SYSTEM_CALLS_H */
