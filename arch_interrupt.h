#pragma once

#include "interrupt.h"

struct interrupt_controller {
  void (*init)();
  void (*log_status)();
  void (*enable_interrupt)(uint8_t irq);

  // Returns the number of an active interrupt, or IRQ_NONE if none are active.
  irq (*get_active_interrupt)();
};

struct interrupt_controller* arch_get_interrupt_controller();
