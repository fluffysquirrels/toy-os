#pragma once

#include "syscalls.h"

// Contains declarations that arch-specific arch_timer.h should implement

void arch_timer_init();
time arch_timer_systemnow();
void arch_timer_clear_deadline();
void arch_timer_set_deadline(time deadline);
void arch_timer_set_timeout(duration_t d);

struct arch_timer {
  void (*init)();
  time (*systemnow)();
  void (*clear_deadline)();
  void (*set_deadline)(time deadline);
  void (*set_timeout)(duration_t d);
};

struct arch_timer *arch_get_timer();
