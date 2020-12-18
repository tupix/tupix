#include <system/user_thread.h>

#include <data/types.h>

#include <std/io.h>
#include <std/log.h>

#define PRINT_N 10
void
user_thread(void* x)
{
	size_t id = *(size_t*)x;
	x += sizeof(id);
	uint32 c = *(uint32*)x;
	x += sizeof(c);
	for (size_t i = 0; i < PRINT_N; ++i) {
		log(DEBUG, "thread %i: %c", id, c);
		kprintf("%c", c);
		// We need a volatile counter so that the loop is not optimized out.
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER * 10; ++i) {}
	}
	log(DEBUG, "thread %i: done", id);
	return;
}
