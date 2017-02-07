#pragma once

#include "kernel.h"
#include "thread.h"

typedef void (*isr_t)(void);

typedef int8_t irq;
#define IRQ_NONE (-1)

void interrupt_init();
void interrupt_set_handler(unsigned char irq, isr_t isr);
void interrupt_enable(unsigned char irq);
void interrupt_handle();
void interrupt_log_status();

// Returns the number of an active interrupt, or IRQ_NONE if none are active.
irq interrupt_get_active();

struct interrupt_controller {
  void (*init)();
  void (*log_status)();
  void (*enable_interrupt)(uint8_t irq);

  // Returns the number of an active interrupt, or IRQ_NONE if none are active.
  irq (*get_active_interrupt)();
};
