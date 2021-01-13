#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#if __ASSEMBLER__

#define KILL_ME 1
#define GET_CHAR 2
#define PUT_CHAR 3
#define WAIT 4
#define CREATE_THREAD 5

#else /* __ASSEMBLER__ */

enum syscall_id {
	KILL_ME       = 1,
	GET_CHAR      = 2,
	PUT_CHAR      = 3,
	WAIT          = 4,
	CREATE_THREAD = 5,
};

char getchar();
void putchar(unsigned char c);

#endif /* __ASSEMBLER__ */

#endif /* SYSTEM_CALLS_H */
