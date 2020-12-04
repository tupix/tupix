#ifndef DRIVER_TIMER_H
#define DRIVER_TIMER_H

#include <std/types.h>

void reset_timer();
bool l_timer_is_interrupting();
void init_local_timer();

#endif /* DRIVER_TIMER_H */
