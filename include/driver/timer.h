#ifndef DRIVER_TIMER_H
#define DRIVER_TIMER_H

#include <data/types.h>

void reset_timer();
bool l_timer_is_interrupting();
void enable_timer();
void disable_timer();
void init_local_timer();

#endif /* DRIVER_TIMER_H */
