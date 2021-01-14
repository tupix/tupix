#ifndef SYSTEM_SCHEDULER_H
#define SYSTEM_SCHEDULER_H

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/thread.h>

void init_scheduler();

struct tcb* schedule_thread(struct tcb thread);
void scheduler_cycle(struct registers* regs, bool decrement);
void kill_current_thread(struct registers* regs);
size_t get_curr_thread_index();
enum thread_state get_cur_thread_state();
void scheduler_push_uart_read(struct registers* regs);
void scheduler_uart_received();
void pause_cur_thread(size_t duration, struct registers* regs);

#endif /* SYSTEM_SCHEDULER_H */
