#ifndef SYSTEM_SCHEDULER_H
#define SYSTEM_SCHEDULER_H

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/thread.h>

void init_scheduler();
void endless_loop();

struct tcb* schedule_thread(struct tcb* thread);
void scheduler_cycle(struct registers* regs);

#endif /* SYSTEM_SCHEDULER_H */
