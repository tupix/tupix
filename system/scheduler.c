#include <system/scheduler.h>

#include <config.h>

#include <arch/armv7/registers.h>

#include <driver/timer.h>

#include <system/assert.h>
#include <system/kutil.h>
#include <system/thread.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

#include <driver/uart.h>

static struct index_queue thread_indices_q;
static struct index_queue free_indices_q;

/* WAITING QUEUES */
// NOTE: When adding queues, do not forget to initialize them in init_scheduler
static struct index_queue sleep_waiting_q;
static struct index_queue char_waiting_q;
/* \WAITING QUEUES */

static struct tcb threads[N_THREADS + 1];
static struct tcb* running_thread;
static struct tcb* null_thread;

static uint32 tid_count;

extern void endless_loop();

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

static void
decrement_waits()
{
	size_t i = 0;
	size_t p = sleep_waiting_q.head;
	size_t thread_idx;

	while (i < sleep_waiting_q.count) {
		thread_idx = sleep_waiting_q.indices[p];
		circle_forward(p, sleep_waiting_q.size);

		if (!--threads[thread_idx].waiting_duration) {
			// Threads wait is done

			size_t res = pop_index(&sleep_waiting_q);
			ASSERTM(res == thread_idx, "Current thread was not popped.");
			// TODO: Push to front?
			push_index(&thread_indices_q, thread_idx);
		} else {
			// We only want to increment here as pop_index decrements
			// waiting_queue.count. This would then mess with our loop
			// condition.
			++i;
		}
	}
	ASSERTM(p == sleep_waiting_q.tail, "Loop did not cycle correctly.");
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

struct tcb*
init_null_thread()
{
	struct tcb null_thread_init = { 0 };
	null_thread_init.regs.pc    = (uint32)&endless_loop;
	null_thread_init.cpsr       = PROCESSOR_MODE_USR;
	null_thread_init.regs.sp =
			(uint32)get_stack_pointer(null_thread_init.index);
	null_thread_init.initialized = true;
	threads[0]                   = null_thread_init;
	return &(threads[0]);
}

void
init_scheduler()
{
	init_queue(&thread_indices_q);
	init_queue(&free_indices_q);

	/* WAITING QUEUES */
	init_queue(&sleep_waiting_q);
	init_queue(&char_waiting_q);
	/* \WAITING QUEUES */

	tid_count = 0;
	// Mark all indices as free
	for (size_t i = 0; i < free_indices_q.size; ++i)
		push_index(&free_indices_q, i + 1);

	null_thread    = init_null_thread();
	running_thread = null_thread;
	klog(LOG, "Scheduler initialized.");
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
		klog(LOG, "No waiting threads. Thread %i continues",
		     running_thread->id);
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
kill_cur_thread(struct registers* regs)
{
	struct tcb* cur_thread = running_thread;
	running_thread         = pop_thread();
	if (!running_thread)
		running_thread = null_thread;

	// discard volatile
	switch_context(regs, NULL, running_thread);

	push_index(&free_indices_q, cur_thread->index);
}

size_t
get_cur_thread_index()
{
	return running_thread->index;
}

enum thread_state
get_cur_thread_state()
{
	return running_thread->state;
}

struct tcb*
push_waiting_thread(struct index_queue* waiting_q, struct registers* regs)
{
	ASSERTM(running_thread != null_thread,
	        "Null thread should loop endlessly and never wait.");

	push_index(waiting_q, running_thread->index);
	struct tcb* thread = running_thread;

	// Do not "re"queue into thread_indices_q.
	// Explicitly switch context to null_thread too as scheduler_cycle might do
	// nothing if there are no other threads and will assume that `regs` match
	// `running_thread->regs`.
	switch_context(regs, thread, null_thread);
	running_thread = null_thread;

	// As the syscall interrupts and resets the current time slice, we do not
	// want to decrement other waiting threads.
	scheduler_cycle(regs, false);
	return thread;
}

/*
 * NOTE: The waiting duration is only decremented on a scheduler cycle based on
 * a timer interrupt and not if a time slice is terminated early. This means a
 * waiting queue might wait (significantly) longer than expected.
 */
void
scheduler_on_sleep(size_t duration, struct registers* regs)
{
	if (!duration)
		return;
	struct tcb* thread       = push_waiting_thread(&sleep_waiting_q, regs);
	thread->waiting_duration = duration;
}

void
scheduler_on_getchar(struct registers* regs)
{
	struct tcb* thread = push_waiting_thread(&char_waiting_q, regs);
	thread->state      = WAITING;
}

void
scheduler_on_char_received()
{
	if (char_waiting_q.count == 0) {
		klog(LOG, "Received char, but uart pop waiting queue is empty.");
		return;
	}

	size_t index = pop_index(&char_waiting_q);
	if (uart_queue_is_emtpy()) {
		// NOTE(Aurel): Accidental irq?
		return;
	}

	struct tcb* thread = &(threads[index]);
	thread->regs.r0    = uart_pop_char();
	thread->state      = READY;

	push_thread(thread);
}
