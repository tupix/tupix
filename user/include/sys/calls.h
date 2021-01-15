#ifndef USER_SYSTEM_CALLS_H
#define USER_SYSTEM_CALLS_H

void exit();
char getchar();
void putchar(unsigned char c);
void wait(unsigned long duration);
void create_thread(void (*func)(void*), const void* args,
                   unsigned long args_size);

#endif /* USER_SYSTEM_CALLS_H */
