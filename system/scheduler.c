#include <config.h>

#include <data/types.h>
#include <system/thread.h>

#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

struct thread_q {
	size_t size;
	size_t tail, head;
	struct tcb threads[N_THREADS];
};

static volatile struct thread_q waiting_q;
static struct tcb running_thread; // TODO: Make pointer and check if volatile

void
init_scheduler()
{
	memset((void*)&waiting_q, 0, sizeof(waiting_q));
	waiting_q.size = sizeof(waiting_q.threads) / sizeof(struct tcb);
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
	struct tcb thread = { 0 };
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
switch_context(struct general_registers* regs, struct tcb* cur)
{
	cur->regs = *regs;
	*regs	  = running_thread.regs;
	// TODO: lr of current thread? only of some mode is restored
}

void
scheduler_cycle(struct general_registers* regs)
{
	struct tcb old_thread = running_thread;
	if (running_thread.id)
		queue(running_thread);

	running_thread = dequeue();
	// switch_context(regs, &old_thread);
}
