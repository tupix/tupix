/**
 * All syscalls are identified by the immediate value of the `svc` command. To
 * call a syscall with a certain id call:
 * ```
 * svc #ID
 * ```
 * NOTE(Aurel): When implementing a new syscall make sure to first implement the
 * kernel side and then define its id with the value defined there.
 *
 * The handler comply to the (C-assembler calling convention for
 * arm32)[https://en.wikipedia.org/wiki/Calling_convention#ARM_(A32)] and thus
 * use `r0` - `r3` for argument passing and `r0` as the return value. This way
 * the assembler functions on the user side do not need to do anything but call
 * the syscall, which then reads its arguments from the registers and puts the
 * return value into `r0`. This way, the calling C-function sees that return
 * value as what the assembler syscall returned.
 *
 * The mode-change is completely hidden to the user-programmer.
 */

#define EXIT 1
#define GET_CHAR 2
#define PUT_CHAR 3
#define WAIT 4
#define FORK 5

.global exit
.global getchar
.global putchar
.global wait
.global fork
.global create_thread

exit:
    svc #EXIT
    bx lr

getchar:
    svc #GET_CHAR
    bx lr

putchar:
    svc #PUT_CHAR
    bx lr

wait:
    svc #WAIT
    bx lr

fork:
    svc #FORK
    bx lr
