#include <system/scheduler.h>

#include <config.h>

#include <arch/armv7/registers.h>

#include <driver/timer.h>

#include <system/assert.h>
#include <system/entry.h>
#include <system/thread.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

#include <driver/uart.h>

struct index_queue {
	size_t size, count;
	size_t tail, head;
	size_t indices[N_THREADS];
};

static struct index_queue thread_indices_q;
static struct index_queue free_indices_q;

static struct index_queue wait_uart_read_index_q;

static struct tcb threads[N_THREADS + 1];
static struct tcb* running_thread;
static struct tcb* null_thread;

static uint32 tid_count;

// NOTE: When adding queues, do not forget to initialize them in init_scheduler
static struct index_queue waiting_queue;

/*
 * Null the struct but set the size
 */
static void
init_queue(struct index_queue* q)
{
	kmemset(q, 0, sizeof(*q));
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
		klog(WARNING, "Invalid index_queue (NULL)");
		return -1;
	} else if (index <= 0 || index > q->size) {
		klog(WARNING, "Index %i out of bounds", index);
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
		klog(WARNING, "Invalid index_queue (NULL)");
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
		klog(WARNING, "Invalid thread (NULL)");
		return NULL;
	} else if (thread == &(threads[0])) {
		klog(WARNING, "Trying to push null-thread");
		return NULL;
	}

	// Push thread
	ssize_t index = push_index(&thread_indices_q, thread->index);
	if (!index) {
		klog(LOG, "Thread queue full");
		return NULL;
	} else if (0 > index) {
		return NULL; // Other error
	}

	return &(threads[thread->index]);
}

static struct tcb*
pop_thread()
{
	if (!thread_indices_q.count) {
		klog(LOG, "thread_indices_q is empty.");
		return NULL;
	}

	ssize_t index;
	size_t i;
	for (i = 0; i < thread_indices_q.count; ++i) {
		index = pop_index(&thread_indices_q);
		if (!index) {
			klog(LOG, "Thread queue empty");
			return NULL;
		} else if (0 > index) {
			return NULL; // Other error
		}

		if (threads[index].initialized) {
			break;
		} else {
			klog(LOG, "Thread not initialized; getting next one");
			push_index(&thread_indices_q, index);
		}
	}
	// NOTE: Through a successful pop the count would decrease. Thus we need
	// 'greater' and not 'greater or equal' for the case that we pop the last
	// thread in the queue.
	if (i > thread_indices_q.count) {
		klog(LOG, "No thread is initialized, returning null-thread");
		return NULL;
	}

	return &(threads[index]);
}

// TODO(Aurel): Remove.
static void switch_context(struct registers* regs, struct tcb* old,
                           struct tcb* new);

// TODO(Aurel): Error handling.
void
scheduler_push_uart_read(struct registers* regs)
{
	klog(DEBUG, "Pushing running thread onto waiting queue...");
	struct tcb* thread = running_thread;

	push_index(&wait_uart_read_index_q, thread->index);
	thread->state = WAITING;

	klog(DEBUG, "Cycling scheduler.");
	running_thread = null_thread;
	switch_context(regs, thread, null_thread);
	scheduler_cycle(regs, false);
}

void
scheduler_uart_received()
{
	if (wait_uart_read_index_q.count == 0) {
		klog(LOG, "Received char, but uart pop waiting queue is empty.");
		return;
	}

	size_t index = pop_index(&wait_uart_read_index_q);
	if (uart_queue_is_emtpy()) {
		// NOTE(Aurel): Accidental irq?
		return;
	}

	// TODO(Aurel): Align c?
	char c = uart_pop_char();

	struct tcb* thread = &(threads[index]);
	thread->regs.r0    = c;
	thread->state      = READY;

	push_thread(thread);
}

static void
decrement_waits()
{
	size_t i = 0;
	size_t p = waiting_queue.head;
	size_t thread_idx;

	while (i < waiting_queue.count) {
		thread_idx = waiting_queue.indices[p];
		circle_forward(p, waiting_queue.size);

		if (!--threads[thread_idx].waiting_for) {
			// Threads wait is done

			// TODO: ASSERT(result == thread_idx)
			pop_index(&waiting_queue);
			// TODO: Push to front?
			push_index(&thread_indices_q, thread_idx);
		} else {
			// We only want to increment here as pop_index decrements
			// waiting_queue.count. This would then mess with our loop
			// condition.
			++i;
		}
	}
	// TODO: ASSERT(p == waiting_queue->tail)
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

	/* WAITING QUEUES */
	init_queue(&waiting_queue);
	init_queue(&wait_uart_read_index_q);
	/* \WAITING QUEUES */

	tid_count = 0;
	// Mark all indices as free
	for (size_t i = 0; i < free_indices_q.size; ++i)
		push_index(&free_indices_q, i + 1);

	null_thread    = init_null_thread();
	running_thread = null_thread;
	klog(LOG, "Initialized");
}

/*
 * Put given thread into scheduled queue.
 * The id of the thread is set in this function and a value of 0 indicates a
 * full queue.
 *
 * @return NULL if the thread could not be pushed.
 */
struct tcb*
schedule_thread(struct tcb thread)
{
	ssize_t index = pop_index(&free_indices_q);
	if (!index) {
		klog(WARNING, "No available thread indices");
		return NULL;
	} else if (0 > index) {
		return NULL; // Other error
	}
	thread.id                 = ++tid_count;
	thread.index              = index;
	threads[thread.index]     = thread;
	struct tcb* queued_thread = push_thread(&thread);
	if (queued_thread) {
		klog(LOG, "New thread: %i.", queued_thread->id);
	} else {
		// Make index available again
		push_index(&free_indices_q, index);
	}
	return queued_thread;
}

static void
switch_context(struct registers* regs, struct tcb* old, struct tcb* new)
{
	if (!regs) {
		klog(ERROR, "regs points to NULL");
		return;
	}
	// We never want to change the null_threads register
	if (old && old != null_thread) {
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
	kprintf("\n");
}

void
scheduler_cycle(struct registers* regs, bool decrement)
{
	klog(LOG, "Cycling...");

	// Before doing anything else, decrement the waiting times of all waiting
	// threads and eventually readd them to the thread_indices_q.
	if (decrement)
		decrement_waits();

	// Continue if no other threads are waiting.
	if (!thread_indices_q.count) {
		klog(LOG, "No waiting threads. Thread %i continues", running_thread->id);
		return;
	}

	/*
	 * NOTE: Pop before pushing as the other way around will not work when
	 * the queue is full.
	 */
	struct tcb* old_thread = running_thread;
	running_thread         = pop_thread();
	if (!running_thread) {
		klog(LOG, "Cannot pop thread. Thread %i continues", old_thread->id);
		running_thread = old_thread;
		return;
	}

	if (old_thread != null_thread) {
		if (!push_thread(old_thread)) {
			klog(ERROR, "Could not push old thread back. Losing the thread!");
			return;
		}
		switch_context(regs, old_thread, running_thread);
	} else {
		// Only set register values, do not modify null-thread
		switch_context(regs, NULL, running_thread);
	}

	klog(LOG, "Running thread: %i", running_thread->id);
}

void
kill_current_thread(struct registers* regs)
{
	struct tcb* current_thread = running_thread;
	running_thread             = pop_thread();
	if (!running_thread)
		running_thread = null_thread;

	// discard volatile
	switch_context(regs, NULL, running_thread);

	push_index(&free_indices_q, current_thread->index);
}

size_t
get_curr_thread_index()
{
	return running_thread->index;
}

enum thread_state
get_cur_thread_state()
{
	return running_thread->state;
}

// NOTE: Currently the waiting times are only decremented on cycle, so when the
// interrupt fired and the timer run at least one full time slice. Beware that
// this does not happen in kill_current_thread.
void
pause_cur_thread(size_t duration, struct registers* regs)
{
	// TODO: Assert(running_thread != null_thread)
	running_thread->waiting_for = duration;
	push_index(&waiting_queue, running_thread->index);

	// Do not requeue into thread_indices_q.
	// Explicitly switch context to null_thread too as scheduler_cycle might do
	// nothing if there are no other threads and will assume that `regs` match
	// `running_thread->regs`.
	switch_context(regs, running_thread, null_thread);
	running_thread = null_thread;

	// As the syscall interrupts and resets the current time slice, we do not
	// want to decrement other waiting threads.
	scheduler_cycle(regs, false);
}
