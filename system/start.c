#include <system/io.h>

void start_kernel(void)
{
	// yellow_on();
	// for (int i = 0; i < 10; i++) {
	// 	for (int i = 0; i < 10; i++) {
	// 		kputchar('a' + i);
	// 	}
	// 	kputchar('\n');
	// }

	kprintf("foo %c%c%c %s %%\n", 98, 'a', 'r', "baz");
	kprintf("%x %i %u\n", 31, -31, 31);
	kprintf("%010p\n", &start_kernel);
}
