#include <system/scheduler.h>

#include <arch/armv7/registers.h>

#include <config.h>

#include <data/types.h>
#include <system/assert.h>
#include <system/thread.h>
#include <system/entry.h>

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
static struct tcb* null_thread;

static uint32 tid_count;

/*
 * Null the struct but set the size
 */
static void
init_queue(struct index_queue* q)
{
	memset(q, 0, sizeof(*q));
	q->size = sizeof(q->indices) / sizeof(*(q->indices));
}

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

/*
 * Push index to index_queue.
 *
 * @return 0 if queue is full and -1 on any other fatal error.
 */
static ssize_t
push_index(struct index_queue* q, size_t index)
{
	if (!q) {
		log(WARNING, "Invalid index_queue (NULL)");
		return -1;
	} else if (index <= 0 || index > q->size) {
		log(WARNING, "Index %i out of bounds", index);
		return -1;
	} else if (q->count >= q->size) {
		return 0;
	}

	q->indices[q->tail] = index;
	circle_forward(q->tail, q->size);
	++(q->count);
	return index;
}

/*
 * Pop index from index_queue.
 *
 * @return 0 if queue is empty and -1 on any other fatal error.
 */
static ssize_t
pop_index(struct index_queue* q)
{
	if (!q) {
		log(WARNING, "Invalid index_queue (NULL)");
		return -1;
	} else if (!q->count) {
		return 0;
	}

	size_t index = q->indices[q->head];
	circle_forward(q->head, q->size);
	--(q->count);
	return index;
}

/*
 * Push thread onto thread queue.
 *
 * @return NULL on errors.
 */
static struct tcb*
push_thread(struct tcb* thread)
{
	if (!thread) {
		log(WARNING, "Invalid thread (NULL)");
		return NULL;
	} else if (thread == &(threads[0])) {
		log(WARNING, "Trying to push null-thread");
		return NULL;
	}

	// Push thread
	ssize_t index = push_index(&thread_indices_q, thread->index);
	if (!index) {
		log(WARNING, "Thread queue full");
		return NULL;
	} else if (0 > index) {
		return NULL; // Other error
	}

	return &(threads[thread->index]);
}

static struct tcb*
pop_thread()
{
	ssize_t index;
	size_t i;
	for (i = 0; i < thread_indices_q.count; ++i) {
		index = pop_index(&thread_indices_q);
		if (!index) {
			log(WARNING, "Thread queue empty");
			return NULL;
		} else if (0 > index) {
			return NULL; // Other error
		}

		if (threads[index].initialized) {
			break;
		} else {
			log(LOG, "Thread not initialized; getting next one");
			push_index(&thread_indices_q, index);
		}
	}
	// NOTE: Through a successful pop the count would decrease. Thus we need
	// 'greater' and not 'greater or equal' for the case that we pop the last
	// thread in the queue.
	if (i > thread_indices_q.count) {
		log(WARNING, "No thread is initialized, returning null-thread");
		return NULL;
	}

	return &(threads[index]);
}

struct tcb*
init_null_thread()
{
	struct tcb null_thread_init = { 0 };
	null_thread_init.regs.pc    = (uint32)&endless_loop;
	null_thread_init.regs.sp =
			(uint32)get_stack_pointer(null_thread_init.index);
	threads[0] = null_thread_init;
	return &(threads[0]);
}

void
init_scheduler()
{
	init_queue(&thread_indices_q);
	init_queue(&free_indices_q);
	tid_count = 0;
	// Mark all indices as free
	for (size_t i = 0; i < free_indices_q.size; ++i)
		push_index(&free_indices_q, i + 1);

	null_thread    = init_null_thread();
	running_thread = null_thread;
	// TODO: Switch context?
	log(LOG, "Initialized");
}

/*
 * Put given thread into scheduled queue.
 * The id of the thread is set in this function and a value of 0 indicates a
 * full queue.
 *
 * @return NULL if the thread could not be pushed.
 */
struct tcb*
schedule_thread(struct tcb* thread)
{
	ssize_t index = pop_index(&free_indices_q);
	if (!index) {
		log(WARNING, "No available thread indices");
		return NULL;
	} else if (0 > index) {
		return NULL; // Other error
	}
	thread->id                = ++tid_count;
	thread->index             = index;
	threads[thread->index]    = *thread;
	struct tcb* queued_thread = push_thread(thread);
	if (queued_thread) {
		log(LOG, "New thread: %i.", queued_thread->id);
	} else {
		// Make index available again
		push_index(&free_indices_q, index);
	}
	return queued_thread;
}

static void
switch_context(struct registers* regs, struct tcb* old, struct tcb* new)
{
	if (old) {
		old->regs    = regs->gr;
		old->regs.lr = regs->usr_lr;
		old->regs.sp = regs->usr_sp;
		old->regs.pc = regs->gr.lr;
		old->cpsr    = regs->spsr;
	}
	if (new) {
		regs->gr     = new->regs;
		regs->usr_lr = new->regs.lr;
		regs->usr_sp = new->regs.sp;
		regs->gr.lr  = new->regs.pc;
		regs->spsr   = new->cpsr;
	}
	// TODO: Are we loosing the lr when overwriting it with the function pointer
	// in thread_create? Do we need to safe the previous lr?
}

void
scheduler_cycle(struct registers* regs)
{
	log(LOG, "Cycling...");
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
	if (!running_thread) {
		log(WARNING, "Cannot pop thread. Thread %i continues", old_thread->id);
		running_thread = old_thread;
		return;
	}

	if (old_thread->id) {
		if (!push_thread(old_thread)) {
			log(ERROR, "Could not push old thread back. Losing the thread!");
			return; // TODO: PANIC?
		}
		switch_context(regs, old_thread, running_thread);
	} else {
		// Only set register values, do not modify null-thread
		switch_context(regs, NULL, running_thread);
	}

	log(LOG, "Running thread: %i", running_thread->id);
	kprintf("!");
}
