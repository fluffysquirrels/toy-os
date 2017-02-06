#include "interrupt.h"

#include "arch_interrupt_common.h"
#include "arch_interrupt.h"
#include "synchronous_console.h"
#include "util.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

static isr_t interrupt_handlers[PIC_INTNUM_COUNT];

struct interrupt_controller *ic;

void interrupt_init() {
  ic = arch_get_interrupt_controller();
  ic->init();
}

void interrupt_handle() {
  sc_LOG_IF(TRACE_INTERRUPTS, "start");

#if TRACE_INTERRUPTS
  ic->log_status();
#endif // TRACE_INTERRUPTS

  irq irq = interrupt_get_active();

  sc_LOGF_IF(TRACE_INTERRUPTS, "irq = %x", (uint32_t) irq);

  ASSERT(irq != IRQ_NONE);
  ASSERT(irq >= 0);
  ASSERT(irq < PIC_INTNUM_COUNT);

  isr_t isr = interrupt_handlers[irq];
  if(!isr) {
    PANICF("Interrupt with no handler irq = %x", irq);
    // Not reached.
    return;
  }

  sc_LOGF_IF(TRACE_INTERRUPTS, "calling handler @%x", (unsigned int) isr);

  isr();

#if TRACE_INTERRUPTS
  sc_puts("interrupt_handle() returned from handler\n");
  ic->log_status();
  sc_puts("\n");
#endif // TRACE_INTERRUPTS
}

void interrupt_set_handler(unsigned char irq, isr_t isr) {
  sc_LOGF_IF(TRACE_INTERRUPTS,
    "irq = %x   isr = %x", irq, (unsigned int) isr);

  ASSERT(interrupt_handlers[irq] == NULL);
  interrupt_handlers[irq] = isr;
}

void interrupt_enable(unsigned char irq) {
  // Enable interrupt on controller

  sc_LOGF_IF(TRACE_INTERRUPTS, "irq = %x", irq);

  ic->enable_interrupt(irq);
}

irq interrupt_get_active() {
  return ic->get_active_interrupt();
}
