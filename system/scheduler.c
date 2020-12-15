#include <config.h>

#include <data/types.h>
#include <system/thread.h>

#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

struct thread_q {
	size_t size, count;
	size_t tail, head;
	struct tcb threads[N_THREADS];
};

static volatile struct thread_q waiting_q;
static struct tcb running_thread = { 0 };

void
init_scheduler()
{
	memset((void*)&waiting_q, 0, sizeof(waiting_q));
	waiting_q.size = sizeof(waiting_q.threads) / sizeof(struct tcb);
}

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

static struct tcb*
queue(struct tcb* thread)
{
	if (waiting_q.count >= waiting_q.size) {
		log(WARNING, "Thread queue full.");
		return thread;
	}
	waiting_q.threads[waiting_q.head] = *thread;
	struct tcb* ret = (struct tcb*)waiting_q.threads + waiting_q.head;
	circle_forward(waiting_q.head, waiting_q.size);
	++waiting_q.count;
	log(LOG, "Queued thread %i.", thread->id);
	return ret;
}

static struct tcb
dequeue()
{
	struct tcb thread = { 0 };
	if (!waiting_q.count) {
		log(LOG, "Thread queue empty. Returning nulled thread.");
		return thread;
	}

	thread = waiting_q.threads[waiting_q.tail];
	circle_forward(waiting_q.tail, waiting_q.size);
	--waiting_q.count;
	log(LOG, "Dequeued thread %i.", thread.id);
	return thread;
}

/*
 * Put given thread into scheduled queue.
 * The id of the thread is set in this function and a value of 0 indicates a
 * full queue.
 */
struct tcb*
schedule_thread(struct tcb* thread)
{
	if (waiting_q.count >= waiting_q.size) {
		log(WARNING, "Thread queue full.");
		thread->id = 0;
		return thread;
	}
	// TODO: What do we do if the threads stop being continues? For example when
	// thread with id 1 exist. `count + 1` would exist then.
	thread->id = waiting_q.count + 1;
	log(LOG, "New thread: %i.", thread->id);
	return queue(thread);
}

static void
switch_context(struct general_registers* regs, struct tcb* cur)
{
	cur->regs = *regs;
	*regs	  = running_thread.regs;
	// TODO: lr of current thread? only of same mode is restored
}

void
scheduler_cycle(struct general_registers* regs)
{
	// Continue if no other threads are waiting.
	if (!waiting_q.count) {
		log(LOG, "No waiting threads. Thread %i continues", running_thread.id);
		return;
	}

	// NOTE: Dequeue before queueing as the other way around will not work when
	// the queue is full.
	struct tcb old_thread = running_thread;
	running_thread		  = dequeue();
	if (old_thread.id)
		queue(&old_thread);
	log(LOG, "New running thread: %i", running_thread.id);

	// switch_context(regs, &old_thread);
}
