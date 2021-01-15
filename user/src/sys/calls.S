#define KILL_ME 1
#define GET_CHAR 2
#define PUT_CHAR 3
#define WAIT 4
#define CREATE_THREAD 5

.global kill_me
.global getchar
.global putchar
.global wait
.global create_thread

kill_me:
    svc #KILL_ME
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
