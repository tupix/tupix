#ifndef USER_SYSTEM_CALLS_H
#define USER_SYSTEM_CALLS_H

#include <data/types.h>

/*
 * See user/sys/calls.S for implementation
 *
 * All syscalls are identified over the immediate value of the `svc` command.
 * So to call a syscall with a certain id it can be done by:
 * svc #ID
 * See include/system/calls.S for all available IDs.
 *
 * Arguments are passed by storing the values in the r0 - r3 register before
 * calling. The exception handler then reads those out.
 * If the syscalls needs to return the handler writes its value into r0 of the
 * calling mode.
 *
 * Because of that convention the assembler function have to do nothing but
 * calling the syscall and together with this header C will do the rest.
 */

void exit();
char getchar();
void putchar(unsigned char c);
void wait(uint32 duration);
void create_thread(void (*func)(void*), const void* args, uint32 args_size);

#endif /* USER_SYSTEM_CALLS_H */
