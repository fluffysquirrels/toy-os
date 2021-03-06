#include "interrupt.h"

#include "arch_interrupt.h"
#include "arch_interrupt_numbers.h"
#include "context_switch.h"
#include "console.h"
#include "util.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

static isr_t interrupt_handlers[INTNUM_MAX + 1];

struct interrupt_controller *ic;

void interrupt_init() {
  ic = arch_get_interrupt_controller();
  ic->init();
}

void interrupt_log_status() {
  ic->log_status();
}

void interrupt_handle() {
  LOG_IF(TRACE_INTERRUPTS, "start");

  irq irq = interrupt_get_active();

  LOGF_IF(TRACE_INTERRUPTS, "irq = %x, %u", (uint32_t) irq, (uint32_t) irq);

  ASSERT(irq != IRQ_NONE);
  ASSERT(irq >= 0);
  ASSERT(irq <= INTNUM_MAX);

  isr_t isr = interrupt_handlers[irq];
  if(!isr) {
    PANICF("Interrupt with no handler irq = %x", irq);
    // Not reached.
    return;
  }

  LOGF_IF(TRACE_INTERRUPTS, "calling handler @%x", (unsigned int) isr);

  isr();

#if TRACE_INTERRUPTS
  LOG("returned from handler\n");
  interrupt_log_status();
#endif // TRACE_INTERRUPTS
}

void interrupt_set_handler(unsigned char irq, isr_t isr) {
  LOGF_IF(TRACE_INTERRUPTS,
             "\n"
             "  irq = %x, %u\n"
             "  isr = %x",
             irq, irq, (unsigned int) isr);

  ASSERT(irq <= INTNUM_MAX);
  ASSERT(interrupt_handlers[irq] == NULL);
  interrupt_handlers[irq] = isr;
}

void interrupt_enable(unsigned char irq) {
  // Enable interrupt on controller

  LOGF_IF(TRACE_INTERRUPTS, "irq = %x, %u", irq, irq);
#if TRACE_INTERRUPTS
  interrupt_log_status();
#endif // TRACE_INTERRUPTS

  ic->enable_interrupt(irq);

#if TRACE_INTERRUPTS
  LOG("Enabled");
  interrupt_log_status();
#endif // TRACE_INTERRUPTS
}

irq interrupt_get_active() {
#if TRACE_INTERRUPTS
  interrupt_log_status();
#endif // TRACE_INTERRUPTS

  return ic->get_active_interrupt();
}
