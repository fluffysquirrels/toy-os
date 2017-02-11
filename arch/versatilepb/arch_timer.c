#include "arch_timer.h"
#include "timer_versatilepb.h"

struct arch_timer *arch_get_timer() {
  return timer_versatilepb_get_arch_timer();
}
