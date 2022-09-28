#ifndef SYSTEM_SCHEDULER_H
#define SYSTEM_SCHEDULER_H

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/thread.h>

void init_scheduler();

struct pcb* scheduler_register_process(struct pcb* process);

/*
 * Register thread.
 *
 * @return a pointer to the new memory location.
 */
struct tcb* scheduler_register_thread(struct tcb* thread);

void scheduler_cycle(struct registers* regs, bool decrement);
void kill_cur_thread(struct registers* regs);
size_t get_cur_thread_index();

/* NOTE: Be careful that the scheduler is not running when calling these two to
 *       prevent a race condition.
 */
struct tcb* get_cur_thread();
struct pcb* get_cur_process();

enum thread_state get_cur_thread_state();
void scheduler_on_getchar(struct registers* regs);
void scheduler_on_char_received();

/*
 * NOTE: The waiting duration is only decremented on a scheduler cycle based on
 * a timer interrupt and not if a time slice is terminated early. This means a
 * waiting queue might wait (significantly) longer than expected.
 */
void scheduler_on_sleep(size_t duration, struct registers* regs);

#endif /* SYSTEM_SCHEDULER_H */
