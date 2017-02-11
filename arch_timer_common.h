#pragma once

#include "syscalls.h"

// Contains declarations that arch-specific arch_timer.h should implement

struct arch_timer {
  void (*init)();
  time (*systemnow)();
  void (*clear_deadline)();
  void (*set_deadline)(time deadline);
  void (*set_timeout)(duration_t d);
};

struct arch_timer *arch_get_timer();
