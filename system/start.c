#include <led.h>
#include <serial.h>

void start_kernel(void)
{
	// yellow_on();
	kputchar('a');
	kputchar('\n');
}
