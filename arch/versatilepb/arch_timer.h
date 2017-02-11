#pragma once

#include "arch_timer_common.h"
#include "syscalls.h"
#include "timer_versatilepb.h"

void arch_timer_init() {
  timer_versatilepb_init();
}

time arch_timer_systemnow() {
  return timer_versatilepb_systemnow();
}

void arch_timer_set_timeout(duration_t d) {
  timer_versatilepb_set_timeout(d);
}

void arch_timer_set_deadline(time t) {
  timer_versatilepb_set_deadline(t);
}

struct arch_timer *arch_get_timer() {
  return timer_versatilepb_get_arch_timer();
}
