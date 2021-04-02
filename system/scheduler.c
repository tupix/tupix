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
STATIC_INDEX_QUEUE(process_indices, N_PROCESSES);
STATIC_INDEX_QUEUE(thread_indices, N_THREADS);

/* WAITING QUEUES */
// NOTE: When adding queues, do not forget to initialize them in init_scheduler
STATIC_INDEX_QUEUE(sleep_waiting, N_THREADS);
STATIC_INDEX_QUEUE(char_waiting, N_THREADS);
/* \WAITING QUEUES */

static struct pcb processes[N_PROCESSES + 1];
static struct tcb threads[N_THREADS + 1];
static struct tcb* running_thread;
static struct tcb* null_thread;

extern void endless_loop();

static size_t
get_global_thread_index(struct tcb* t)
{
	/*
	 * NOTE: A small visualization of how the threads are put into the
	 * threads-table based on their processes index:
	 *
	 * processes        [ 0 | 1     | 2     | 3     | ... ]
	 * threads (local)  [ 0 | 1 2 3 | 1 2 3 | 1 2 3 | ... ]
	 * threads (global) [ 0 | 1 2 3 | 4 5 6 | 7 8 9 | ... ]
	 *
	 * (with N_THREADS_PER_PROCESS = 3)
	 */

	// The formula only works with indices greater than zero
	if (!t->process->index)
		return 0;

	return (t->process->index - 1) * N_THREADS_PER_PROCESS + t->index;
}

static void
decrement_waits()
{
	size_t i = 0;
	size_t p = sleep_waiting_q.head;
	ssize_t thread_idx;

	while (i < sleep_waiting_q.count) {
		thread_idx = sleep_waiting_q.indices[p];
		circle_forward(p, sleep_waiting_q.size);

		if (!--threads[thread_idx].waiting_duration) {
			// threads wait is done

			ssize_t res = pop_index(&sleep_waiting_q);
			ASSERTM(res == thread_idx, "Current thread was not popped.");
			// TODO: Push to front?
			push_index(&thread_indices_q, thread_idx);
		} else {
			/*
			 * NOTE: We only want to increment here as pop_index decrements
			 * waiting_queue.count. This would then mess with our loop
			 * condition.
			 */
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
	// never change the null_threads registers
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
	switch_memory(new->process->l2_table);
}

struct tcb*
init_null_thread()
{
	struct pcb null_process     = { 0 };
	struct tcb null_thread_init = { 0 };
	null_thread_init.regs.pc    = (uint32)&endless_loop;
	null_thread_init.cpsr       = PROCESSOR_MODE_USR;
	null_thread_init.regs.sp =
			(uint32)get_stack_pointer(null_thread_init.index);

	threads[0] = null_thread_init;

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
	INIT_INDEX_QUEUE(process_indices);
	INIT_INDEX_QUEUE(thread_indices);

	/* WAITING QUEUES */
	INIT_INDEX_QUEUE(sleep_waiting);
	INIT_INDEX_QUEUE(char_waiting);
	/* \WAITING QUEUES */

	MARK_ALL_FREE(free_process_indices_q);

	null_thread    = init_null_thread();
	running_thread = null_thread;
	klog(LOG, "Scheduler initialized.");
}

struct pcb*
scheduler_register_process(struct pcb* process)
{
	ssize_t index = pop_index(&free_process_indices_q);
	if (index < 0)
		return NULL;

	process->index            = index;
	processes[process->index] = *process;
	return &processes[process->index];
}
/*
 * Register thread.
 *
 * @return a pointer to the new memory location.
 */
struct tcb*
scheduler_register_thread(struct tcb* thread)
{
	size_t index = get_global_thread_index(thread);
	if (push_index(&thread_indices_q, index) < 0)
		return NULL;

	threads[index] = *thread;
	return &(threads[index]);
}

void
scheduler_cycle(struct registers* regs, bool decrement)
{
	klog(LOG, "Cycling...");

	/*
	 * NOTE: Before doing anything else, decrement the waiting times of all
	 * waiting threads and eventually push them back into the thread_indices_q.
	 */
	if (decrement)
		decrement_waits();

	// continue if no other threads are waiting.
	if (!thread_indices_q.count) {
		klog(LOG,
		     "No waiting threads. Thread (p%u,t%u)(pidx%u,tidx%u) continues...",
		     running_thread->process->pid, running_thread->tid,
		     running_thread->process->index, running_thread->index);
		return;
	}

	/*
	 * NOTE: Pop before pushing as the other way around will not work when
	 * the queue is full.
	 */
	struct tcb* old_thread = running_thread;
	ssize_t index          = pop_index(&thread_indices_q);
	if (index < 0) {
		klog(LOG, "Cannot pop thread. (p%u,t%u)(pidx%u,tidx%u) continues...",
		     old_thread->process->pid, old_thread->tid,
		     running_thread->process->index, running_thread->index);
		running_thread = old_thread;
		return;
	}
	running_thread = &(threads[index]);

	if (old_thread != null_thread) {
		if (push_index(&thread_indices_q, old_thread->index) < 0) {
			// TODO: Do we need to do something with the free_indices_q?
			klog(ERROR, "Could not push old thread back. Losing the thread!");
			return;
		}
	} else {
		// only set register values; do not modify null-thread
		old_thread = NULL;
	}
	switch_context(regs, old_thread, running_thread);

	klog(LOG, "Running thread: (p%u,t%u)(pidx%u,tidx%u)",
	     running_thread->process->pid, running_thread->tid,
	     running_thread->process->index, running_thread->index);
}

void
kill_cur_thread(struct registers* regs)
{
	struct tcb* cur_thread  = get_cur_thread();
	struct pcb* cur_process = get_cur_process();
	ssize_t index           = pop_index(&thread_indices_q);
	if (index <= 0)
		running_thread = null_thread;
	else
		running_thread = &(threads[index]);

	switch_context(regs, NULL, running_thread);
	// TODO(Aurel): Clear memory section. Or should this be left to user
	// programs?

	// make thread index available again in process
	push_index(&(cur_process->free_indices_q), cur_thread->index);
	klog(LOG, "Killed thread (p%u,t%u)(pidx%u,tidx%u).", cur_process->pid,
	     cur_thread->tid, cur_process->index, cur_thread->index);

	if (cur_process->free_indices_q.count >= cur_process->free_indices_q.size) {
		klog(LOG, "No more threads in process. Killing process (p%u,pidx%u).",
		     cur_process->pid, cur_process->index);
		push_index(&free_process_indices_q, cur_process->index);
	}
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

	push_index(waiting_q, get_global_thread_index(running_thread));
	struct tcb* thread = running_thread;

	/*
	 * NOTE: Do not push back into thread_indices_q.
	 * Explicitly switch context to null_thread too as scheduler_cycle might do
	 * nothing if there are no other threads and will assume that `regs` match
	 * `running_thread->regs`.
	 */
	switch_context(regs, thread, null_thread);
	running_thread = null_thread;

	/*
	 * NOTE: As the syscall interrupts and resets the current time slice, we do
	 * not want to decrement the waiting threads.
	 */
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

	ssize_t index = pop_index(&char_waiting_q);
	if (index < 0)
		return;

	if (uart_queue_is_emtpy()) {
		// NOTE(Aurel): Accidental irq?
		return;
	}

	struct tcb* thread = &(threads[index]);
	thread->regs.r0    = uart_pop_char();
	thread->state      = READY;

	push_index(&thread_indices_q, index);
}
