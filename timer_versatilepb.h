#pragma once

#include "syscalls.h"

void timer_versatilepb_init();
void timer_versatilepb_rtc_tick();
time timer_versatilepb_systemnow();

void timer_versatilepb_set_timeout(duration_t d);
