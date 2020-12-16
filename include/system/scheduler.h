#ifndef SYSTEM_SCHEDULER_H
#define SYSTEM_SCHEDULER_H

#include <arch/armv7/registers.h>

#include <data/types.h>
#include <system/thread.h>

/*
 * NOTE(Aurel): Simple scheduler with super simple threads.
 * To create a thread call `create_thread`. It will create a super simple thread
 * with a unique id and a callback to a simple function which prints the threads
 * id for a short while.
 * To start the scheduler initialize it first by calling `init_scheduler` and
 * then start it. It will dequeue the threads, run them and then queue them
 * again causing an (purposeful) infinite loop.
 *
 * NOTE(Aurel): Because of the infinite loop and because the scheduler is not
 * yet based on interrupts, you need to create threads before starting the
 * scheduler otherwise it will just dequeue the empty thread and return.
 */

void init_scheduler();
void endless_loop();

struct tcb* schedule_thread(struct tcb* thread);
void scheduler_cycle(struct registers* regs);

#endif /* SYSTEM_SCHEDULER_H */
