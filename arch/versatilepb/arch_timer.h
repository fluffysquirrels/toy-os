#pragma once

#include "syscalls.h"
#include "timer_versatilepb.h"

void arch_timer_init() {
  timer_versatilepb_init();
}

time arch_timer_systemnow() {
  return timer_versatilepb_systemnow();
}
