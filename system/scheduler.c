#include <config.h>

#include <data/types.h>
#include <system/thread.h>

#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

struct thread_q {
	uint32 size;
	uint32 tail, head;
	struct tcb threads[N_THREADS];
};

static volatile struct thread_q waiting_q;
static struct tcb running_thread;

void
init_scheduler()
{
	waiting_q.size = N_THREADS;
}

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

void
queue(struct tcb thread)
{
	waiting_q.threads[waiting_q.head] = thread;
	log(LOG, "Queued thread %i.", thread.id);
	circle_forward(waiting_q.head, waiting_q.size);
	if (waiting_q.head == waiting_q.tail) {
		circle_forward(waiting_q.tail, waiting_q.size);
	}
}

struct tcb
dequeue()
{
	struct tcb thread = {};
	if (waiting_q.head == waiting_q.tail) {
		log(LOG, "Dequeued empty thread. Waiting queue empty.");
		return thread;
	}

	thread = waiting_q.threads[waiting_q.tail];
	circle_forward(waiting_q.tail, waiting_q.size);
	log(LOG, "Dequeued thread %i.", thread.id);
	return thread;
}

void
schedule_thread(struct tcb thread)
{
	queue(thread);
}

void
scheduler_cycle()
{
	if (running_thread.id)
		queue(running_thread);

	running_thread = dequeue();
	if (running_thread.id == 0) {
		// TODO(Aurel): Run default thread.
	} else {
		// TODO(Aurel): Change privileges to threads privileges. Currently the
		// CPU-mode is not changed and we remain in the mode of the interrupt
		// handler, the privileged mode where the I-bit of the CPSR is set, thus
		// masking new IRQs. This is a problem with the context switching,
		// which is not implemented yet, so I think there is nothing that I can
		// do right now.
		(*(running_thread.callback))(&running_thread);
		queue(running_thread);
	}
}
