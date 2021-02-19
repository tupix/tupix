#include <data/types.h>
#include <std/io.h>
#include <sys/calls.h>

#define TEST_UDATA_SEPERATION 0

#if TEST_UDATA_SEPERATION
char PRINT_CHAR;
unsigned int GLOB_COUNTER = 100;
#endif

void
endless_loop()
{
	while (1) {}
}

#if TEST_UDATA_SEPERATION
void
countdown(void* args)
{
	uint32 i = 0;
	if (args) {
		i = *(uint32*)args;
		args += sizeof(i);
	}

	while (GLOB_COUNTER > 0) {
		printf("%c,%i: %i\n", PRINT_CHAR, i, GLOB_COUNTER);
		GLOB_COUNTER--;
		wait(2);
	}
}
#endif /* TEST_UDATA_SEPERATION */

void
user_thread(void* args)
{
	uint32 c = '\\';
	if (args) {
		c = *(uint32*)args;
		args += sizeof(c);
	}

#if TEST_UDATA_SEPERATION
	PRINT_CHAR = c;
	uint32 one = 1, two = 2, three = 3;
	create_thread(&countdown, &one, sizeof(c));
	create_thread(&countdown, &two, sizeof(c));
	create_thread(&countdown, &three, sizeof(c));
#else  /* TEST_UDATA_SEPERATION */
	printf("Hello World! ");
	printf("%c\n", c);
#endif /* TEST_UDATA_SEPERATION */
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
