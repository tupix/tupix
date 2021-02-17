#ifndef SYSTEM_SCHEDULER_H
#define SYSTEM_SCHEDULER_H

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/thread.h>

void init_scheduler();

struct pcb* schedule_process(struct pcb process);
struct tcb* schedule_thread(struct tcb thread);
void scheduler_cycle(struct registers* regs, bool decrement);
void kill_cur_thread(struct registers* regs);
size_t get_cur_thread_index();
struct tcb* get_cur_thread();
struct pcb* get_cur_process();
enum thread_state get_cur_thread_state();
void scheduler_on_getchar(struct registers* regs);
void scheduler_on_char_received();
void scheduler_on_sleep(size_t duration, struct registers* regs);

#endif /* SYSTEM_SCHEDULER_H */
