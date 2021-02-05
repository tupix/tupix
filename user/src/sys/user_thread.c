#include <std/io.h>
#include <sys/calls.h>

void
endless_loop()
{
	while (1) {}
}

void
user_thread(void* args)
{
#if 0
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
#else
	printf("Hello World!");
#endif
}
