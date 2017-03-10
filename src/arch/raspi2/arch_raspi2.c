#include "arch_timer.h"
#include "interrupt_raspi.h"
#include "timer_raspi.h"

struct arch_timer *arch_get_timer() {
  return timer_raspi_get_arch_timer();
}

struct interrupt_controller* arch_get_interrupt_controller() {
  return interrupt_raspi_get_ic();
}
