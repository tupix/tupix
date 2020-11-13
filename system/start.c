#include <config.h>
#include <driver/uart.h>
#include <system/io.h>

void start_kernel(void)
{
	init_uart();

	test_kprintf();

	// yellow_on();
	// for (int i = 0; i < 10; i++) {
	// 	for (int i = 0; i < 10; i++) {
	// 		kputchar('a' + i);
	// 	}
	// 	kputchar('\n');
	// }

	//kprintf("foo %c%c%c %s %%\n", 98, 'a', 'r', "baz");
	//kprintf("%x %i %u\n", 31, -31, 31);
	//kprintf("%010p\n", &start_kernel);

	unsigned int i = 1;
	kprintf("%016p\n", &i);

	while (1) {
		char c = kgetchar();
		kprintf("%c %03i %03u %x %p\n", c, c, c, c, c);
	}
}
