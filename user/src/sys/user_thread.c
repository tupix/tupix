#include <data/types.h>
#include <std/io.h>
#include <sys/calls.h>

extern uint32 _kstacks_start[];

void
endless_loop()
{
	while (1) {}
}

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
		case 'K':  { // read kernel stack
			volatile uint32 x = *_kstacks_start;
			printf("%x\n", x);
			break;
				   }
		case 'g': // write to hardware
			asm("ldr r0, =0x3F201000;");
			break;
		case 'c': // write to user code
			*((int*)&user_thread) = 0;
			break;
		case 's':  { // stack overflow
			while (true) {
				asm("push { r0 }");
			}
			break;
				   }
		case 'u': // unassigned address
			asm("mov r0, #0xF00000");
			break;
		case 'x': // jump to user stack
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
