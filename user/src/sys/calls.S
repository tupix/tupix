#define KILL_ME 1
#define GET_CHAR 2
#define PUT_CHAR 3
#define WAIT 4
#define CREATE_THREAD 5

kill_me:
    svc #KILL_ME

getchar:
    svc #GET_CHAR

putchar:
    svc #PUT_CHAR

wait:
    svc #WAIT

create_thread:
    svc #CREATE_THREAD
