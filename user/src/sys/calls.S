#define EXIT 1
#define GET_CHAR 2
#define PUT_CHAR 3
#define WAIT 4
#define CREATE_THREAD 5

.global exit
.global getchar
.global putchar
.global wait
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

create_thread:
    svc #CREATE_THREAD
    bx lr
