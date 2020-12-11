#ifndef SYSTEM_SCHEDULER_H
#define SYSTEM_SCHEDULER_H

#include <data/types.h>

void thread_create(void (*func)(void*), const void* args, uint32 args_size);

#endif /* SYSTEM_SCHEDULER_H */
