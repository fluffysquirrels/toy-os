#pragma once

#include "syscalls.h"

// arch_timer_common implementation
void timer_versatilepb_init();
time timer_versatilepb_systemnow();
void timer_versatilepb_set_timeout(duration_t d);
void timer_versatilepb_set_deadline(time t);

void timer_versatilepb_rtc_tick();
