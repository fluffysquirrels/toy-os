#include "interrupt.h"

#include "interrupt_pl190.h"
#include "synchronous_console.h"
#include "util.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

static isr_t interrupt_handlers[PIC_INTNUM_COUNT];

void handle_interrupt() {
  sc_LOG_IF(TRACE_INTERRUPTS, "start");

#if TRACE_INTERRUPTS
  interrupt_pl190_log_status();
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
  sc_puts("handle_interrupt() returned from handler\n");
  interrupt_pl190_log_status();
  sc_puts("\n");
#endif // TRACE_INTERRUPTS
}

void set_interrupt_handler(unsigned char irq, isr_t isr) {
  sc_LOGF_IF(TRACE_INTERRUPTS,
    "irq = %x   isr = %x", irq, (unsigned int) isr);

  ASSERT(interrupt_handlers[irq] == NULL);
  interrupt_handlers[irq] = isr;
}

void interrupt_enable(unsigned char irq) {
  // Enable interrupt on controller

  sc_LOGF_IF(TRACE_INTERRUPTS, "irq = %x", irq);

  interrupt_pl190_enable(irq);
}

irq interrupt_get_active() {
  return interrupt_pl190_get_active();
}
