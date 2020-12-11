#include <system/thread.h>

#include <std/log.h>

static uint32 thread_count;

struct tcb
create_thread()
{
	// NOTE(Aurel): thread_id 0 should be reserved for error codes. Thus
	// pre-increment
	struct tcb thread = { ++thread_count, (void*)&run };
	log(LOG, "New thread: %i.", thread.id);
	return thread;
}

#if 0
// TODO(Aurel): This should eventually be the actual call.
void
thread_create(void (*func)(void*), const void* args, uint32 args_size)
{
}
#endif

#define PRINT_N 10
#define BUSY_WAIT_COUNTER_SCHEDULER 10000000
void
run(struct tcb* thread)
{
	log(LOG, "Running thread: %i.", thread->id);
	for (uint32 i = 0; i < PRINT_N; ++i) {
		kprintf("%i", thread->id);
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER_SCHEDULER; ++i) {}
	}
	kprintf("\n");
}
