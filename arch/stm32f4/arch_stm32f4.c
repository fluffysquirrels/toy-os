#include "arch_timer.h"
#include "arch_interrupt.h"
#include "synchronous_console.h"


struct arch_timer *arch_get_timer() {
  PANIC("WIP");
  // Not reached.
  return NULL;
}

struct interrupt_controller* arch_get_interrupt_controller() {
  PANIC("WIP");
}

