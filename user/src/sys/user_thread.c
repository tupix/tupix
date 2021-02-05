#include <data/types.h>
#include <std/io.h>
#include <sys/calls.h>

extern uint32 _kstacks_start[];

void
user_thread(void* args)
{
	uint32 c = '\\';
	if (args) {
		c = *(uint32*)args;
		args += sizeof(c);
	}

	switch (c) {
		case 'n': // read NULL
			asm("ldr r0, =0");
			break;
		case 'p': // jump to NULL
			asm("mov pc, #0");
			break;
		case 'd': // read kernel data
			asm("ldr r0, =0x200004");
			break;
		case 'k': // read kernel code
			asm("ldr r0, =0x100004");
			break;
		case 'K': // read kernel stack
			// asm("ldr r0, =%0" : "=r" (_kstacks_start));
			break;
		case 'g': // write to hardware
			// TODO: UART address
			asm("ldr r0, =0x3F201000;");
			break;
		case 'c': // write to user code
			// asm("str, =user_thread, #0");
			*((int*)&user_thread) = 0;
			break;
		case 's': // stack overflow
			// asm("add sp, #0x404");
			// asm("ldr r0, =sp");
			break;
		case 'u': // unassigned address
			asm("mov r0, #0xF00000");
			break;
		case 'x':
			asm("mov pc, sp");
			break;
		default:
			break;
	}
}

void
main_thread()
{
	uint32 c;
	while (1) {
		c = getchar();
		create_thread(&user_thread, &c, sizeof(c));
	}
}
