#include <system/thread.h>

#include <std/log.h>

static uint32 thread_count = 0;

struct tcb
create_thread()
{
	struct tcb thread = { 0 };
	// NOTE(Aurel): thread_id 0 should be reserved for error codes. Thus
	// pre-increment
	thread.id		= ++thread_count;
	thread.callback = (void*)&run;
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
	for (uint32 i = 0;; ++i) {
		log(LOG, "thread %i: %i", thread->id, i);
		for (volatile uint32 i = 0; i < BUSY_WAIT_COUNTER_SCHEDULER; ++i) {}
	}
}
