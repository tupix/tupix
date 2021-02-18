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

	printf("Hello World! ");
	printf("%c\n", c);
}

void
main_thread()
{
#if 0
	printf("Hello World!\n");
#else
	uint32 c;
	while (1) {
		c = getchar();
		fork(&user_thread, &c, sizeof(c));
	}
#endif
}
