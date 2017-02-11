#include "arch_interrupt.h"
#include "arch_timer.h"
#include "interrupt_pl190.h"
#include "timer_versatilepb.h"

struct arch_timer *arch_get_timer() {
  return timer_versatilepb_get_arch_timer();
}

struct interrupt_controller* arch_get_interrupt_controller() {
  return interrupt_pl190_get_ic();
}
