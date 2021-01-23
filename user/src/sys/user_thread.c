#include <sys/calls.h>

void
user_thread(void* args)
{
	if (args) {
		uint32 c = *(uint32*)args;
		args += sizeof(c);
		putchar(c);
	}
	putchar('f');
	wait(0);
	uint32 c;
	c = getchar();
	putchar(c);
	create_thread(&user_thread, &c, sizeof(c));
	exit();
	putchar('x');
}
