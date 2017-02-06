#pragma once

#include "arch_timer_common.h"
#include "syscalls.h"
#include "timer_raspi.h"

void arch_timer_init() {
  timer_raspi_init();
}

time arch_timer_systemnow() {
  return timer_raspi_systemnow();
}
