#include "arch_timer.h"
#include "timer_raspi.h"

struct arch_timer *arch_get_timer() {
  return timer_raspi_get_arch_timer();
}
