#include <system/scheduler.h>

#include <config.h>

#include <data/types.h>
#include <system/assert.h>
#include <system/thread.h>

#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

struct index_queue {
	size_t size, count;
	size_t tail, head;
	size_t indices[N_THREADS];
};

static struct index_queue thread_indices_q;
static struct index_queue free_indices_q;

static struct tcb threads[N_THREADS + 1];
static struct tcb* running_thread;

static uint32 tid_count;

void
init_queue(struct index_queue* q)
{
	memset((void*)q, 0, sizeof(*q));
	q->size = sizeof(q->indices) / sizeof(*(q->indices));
}

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

size_t
push_index(struct index_queue* q, size_t index)
{
	ASSERTM(q, "Invalid index_queue.");
	ASSERTM(index <= (sizeof(threads) / sizeof(*threads)),
	        "Index %i out of bounds.", index);
	ASSERTM(index > 0, "Index %i not valid (negative).", index);

	// queue is full
	if (q->count >= q->size)
		return 0;

	q->indices[q->tail] = index;
	circle_forward(q->tail, q->size);
	++(q->count);
	return index;
}

size_t
pop_index(struct index_queue* q)
{
	ASSERTM(q, "Invalid index_queue.");

	// queue is empty
	if (q->count == 0)
		return 0;

	size_t index = q->indices[q->head];
	circle_forward(q->head, q->size);
	--(q->count);
	return index;
}

struct tcb*
push_thread(struct tcb* thread)
{
	ASSERTM(thread != &(threads[0]), "Trying to push null-thread.");

	if (push_index(&thread_indices_q, thread->index))
		return &(threads[thread->index]);

	log(WARNING, "Thread queue full.");
	return NULL;
}

struct tcb*
pop_thread()
{
	size_t index = pop_index(&thread_indices_q);
	if (!index) {
		log(LOG, "Thread queue empty.");
		return NULL;
	}

	if (!(threads[index].initialized)) {
		log(LOG, "Thread not initialized. Returning null-thread.");
		return NULL;
	}

	return &(threads[index]);
}

void
init_scheduler()
{
	init_queue(&thread_indices_q);
	init_queue(&free_indices_q);
	for (size_t i = 0; i < N_THREADS; ++i)
		push_index(&free_indices_q, i + 1);

	// TODO(Aurel): Initialize null-thread in some way?
	// NOTE(Aurel): null-thread
	struct tcb null_thread = { 0 };
	threads[0]             = null_thread;

	running_thread = &(threads[0]);
	tid_count      = 0;
	log(LOG, "[SCHEDULER] Initialized");
}

/*
 * Put given thread into scheduled queue.
 * The id of the thread is set in this function and a value of 0 indicates a
 * full queue.
 *
 * @return NULL if there is no room for the new thread in the queue.
 */
struct tcb*
schedule_thread(struct tcb* thread)
{
	// TODO: What do we do if the threads stop being continues? For example when
	// thread with id 1 exist. `count + 1` would exist then.
	thread->id = ++tid_count;

	thread->index          = pop_index(&free_indices_q);
	threads[thread->index] = *thread;
	push_thread(thread);

	log(LOG, "New thread: %i.", thread->id);
	return (struct tcb*)&(threads[thread->index]);
}

static void
switch_context(struct general_registers* regs, struct tcb* cur)
{
	cur->regs = *regs;
	*regs     = running_thread->regs;
	// TODO: Are we loosing the lr when overwriting it with the function pointer
	// in thread_create? Do we need to safe the previous lr?
}

void
scheduler_cycle(struct general_registers* regs)
{
	log(LOG, "[SCHEDULER] Cycling...");
	// Continue if no other threads are waiting.
	if (!thread_indices_q.count) {
		log(LOG, "No waiting threads. Thread %i continues", running_thread->id);
		return;
	}

	/*
	 * NOTE: Pop before queuing as the other way around will not work when
	 * the queue is full.
	 */
	struct tcb* old_thread = running_thread;
	running_thread         = pop_thread();

	// Overwrite pointer with reference to thread in queue if the running thread
	// was not a null thread.
	if (old_thread->id)
		if (!push_thread(old_thread))
			return;

	switch_context(regs, old_thread);

	log(LOG, "Running thread: %i", running_thread->id);
}
