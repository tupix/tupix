#include <system/scheduler.h>

#include <config.h>

#include <arch/armv7/registers.h>

#include <driver/mmu.h>
#include <driver/timer.h>

#include <system/assert.h>
#include <system/kutil.h>
#include <system/process.h>
#include <system/thread.h>

#include <data/types.h>
#include <std/io.h>
#include <std/log.h>
#include <std/mem.h>

#include <driver/uart.h>

//static struct index_queue process_indices_q;
STATIC_INDEX_QUEUE(free_process_indices, N_PROCESSES);
STATIC_INDEX_QUEUE(thread_indices, N_THREADS);
STATIC_INDEX_QUEUE(free_thread_indices, N_THREADS);

/* WAITING QUEUES */
// NOTE: When adding queues, do not forget to initialize them in init_scheduler
STATIC_INDEX_QUEUE(sleep_waiting, N_THREADS);
STATIC_INDEX_QUEUE(char_waiting, N_THREADS);
/* \WAITING QUEUES */

static struct pcb processes[N_PROCESSES + 1];
static struct tcb threads[N_THREADS + 1];
static struct tcb* running_thread;
static struct tcb* null_thread;

static uint32 tid_count;
static uint32 pid_count;

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

	// TODO:
	// The order in the queue is changed, if there is a thread that is not
	// initialized yet. In that case that thread is pushed to the back. Is that
	// unfair behaviour? We could continue to pop & push instead of breaking so
	// that the order is restored.
	// Another question: Is it even possible that there are uninitialized
	// threads in here?
	ssize_t index = -1;
	for (size_t i = 0; i < thread_indices_q.count; ++i) {
		index = pop_index(&thread_indices_q);
		if (!index) {
			klog(LOG, "Thread queue empty");
			return NULL;
		} else if (index < 0) {
			return NULL; // Other error
		}

		if (threads[index].initialized) {
			break;
		} else {
			klog(LOG, "Thread not initialized; getting next one");
			push_index(&thread_indices_q, index);
		}

		index = -1;
	}
	if (index == -1) {
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
	// TODO(Aurel): Is this correct here? Do we always want to switch the
	// memory?
	switch_memory(new->process->pid, new->process->l2_table);
}

struct tcb*
init_null_thread()
{
	// TODO(Aurel): Save null_process permanently in static global.
	struct pcb null_process     = { 0 };
	struct tcb null_thread_init = { 0 };
	null_thread_init.regs.pc    = (uint32)&endless_loop;
	null_thread_init.cpsr       = PROCESSOR_MODE_USR;
	null_thread_init.regs.sp =
			(uint32)get_stack_pointer(null_thread_init.index);
	null_thread_init.initialized = true;
	threads[0]                   = null_thread_init;
	init_thread_memory(null_process.pid, threads[0].index,
	                   null_process.l2_table);
	processes[0]       = null_process;
	threads[0].process = &(processes[0]);
	return &(threads[0]);
}

void
init_scheduler()
{
	INIT_INDEX_QUEUE(free_process_indices);
	INIT_INDEX_QUEUE(thread_indices);
	INIT_INDEX_QUEUE(free_thread_indices);

	/* WAITING QUEUES */
	INIT_INDEX_QUEUE(sleep_waiting);
	INIT_INDEX_QUEUE(char_waiting);
	/* \WAITING QUEUES */

	tid_count = 0;
	pid_count = 0;
	MARK_ALL_FREE(free_process_indices_q);
	MARK_ALL_FREE(free_thread_indices_q);

	null_thread    = init_null_thread();
	running_thread = null_thread;
	klog(LOG, "Scheduler initialized.");
}

struct pcb*
schedule_process(struct pcb process)
{
	ssize_t index = pop_index(&free_process_indices_q);
	if (!index) {
		klog(WARNING, "No available thread indices");
		return NULL;
	} else if (0 > index) {
		return NULL; // Other error
	}
	process.pid              = ++pid_count;
	process.index            = index;
	processes[process.index] = process;
	return &processes[process.index];
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
	ssize_t index = pop_index(&free_thread_indices_q);
	if (!index) {
		klog(WARNING, "No available thread indices");
		return NULL;
	} else if (0 > index) {
		return NULL; // Other error
	}
	thread.tid                = ++tid_count;
	thread.index              = index;
	threads[thread.index]     = thread;
	struct tcb* queued_thread = push_thread(&thread);
	if (queued_thread) {
		klog(LOG, "New thread (p%u,t%u) scheduled.",
		     queued_thread->process->pid, queued_thread->tid);
	} else {
		// Make index available again
		push_index(&free_thread_indices_q, index);
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
		klog(LOG, "No waiting threads. Thread (p%u,t%u) continues",
		     running_thread->process->pid, running_thread->tid);
		return;
	}

	/*
	 * NOTE: Pop before pushing as the other way around will not work when
	 * the queue is full.
	 */
	struct tcb* old_thread = running_thread;
	running_thread         = pop_thread();
	if (!running_thread) {
		klog(LOG, "Cannot pop thread. (p%u,t%u) continues",
		     old_thread->process->pid, old_thread->tid);
		running_thread = old_thread;
		return;
	}

	if (old_thread != null_thread) {
		if (!push_thread(old_thread)) {
			klog(ERROR, "Could not push old thread back. Losing the thread!");
			return;
		}
	} else {
		// Only set register values, do not modify null-thread
		old_thread = NULL;
	}
	switch_context(regs, old_thread, running_thread);

	klog(LOG, "Running thread: (p%u,t%u)", running_thread->process->pid,
	     running_thread->tid);
}

void
kill_cur_thread(struct registers* regs)
{
	struct tcb* cur_thread  = get_cur_thread();
	struct pcb* cur_process = get_cur_process();
	running_thread          = pop_thread();
	if (!running_thread)
		running_thread = null_thread;

	switch_context(regs, NULL, running_thread);
	// TODO(Aurel): Clear memory section.

	if (cur_process->n_threads == 1) {
		klog(LOG, "No more threads in process. Killing process.");
		push_index(&free_process_indices_q, cur_process->index);
	}

	push_index(&free_thread_indices_q, cur_thread->index);
}

size_t
get_cur_thread_index()
{
	return running_thread->index;
}

// NOTE: Be careful that the scheduler is not running when calling this to
// prevent a race condition.
struct tcb*
get_cur_thread()
{
	return running_thread;
}

// NOTE: Be careful that the scheduler is not running when calling this to
// prevent a race condition.
struct pcb*
get_cur_process()
{
	return running_thread->process;
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
