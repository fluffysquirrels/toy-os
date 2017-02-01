#include "interrupt.h"

#include "interrupt_pl190.h"
#include "synchronous_console.h"
#include "util.h"
#include "versatilepb.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

isr_t interrupt_handlers[PIC_INTNUM_COUNT];

void pic_log_status();

unsigned int interrupt_get_status() {
  return *(PIC + VIC_IRQSTATUS);
}

void handle_interrupt() {
  sc_LOG_IF(TRACE_INTERRUPTS, "start");

  unsigned int pic_irqstatus = interrupt_get_status();

#if TRACE_INTERRUPTS
  pic_log_status();
#endif // TRACE_INTERRUPTS

  ASSERT(pic_irqstatus != 0);

  int irq = __builtin_ctz(pic_irqstatus);

  sc_LOGF_IF(TRACE_INTERRUPTS, "irq = %x", irq);

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
  pic_log_status();
  sc_puts("\n");
#endif // TRACE_INTERRUPTS
}

void pic_log_status() {
  sc_puts("pic_log_status()\n");
  sc_printf("  PIC_IRQSTATUS = %x\n", *(PIC + VIC_IRQSTATUS));
  sc_printf("  PIC_RAWINTR   = %x\n", *(PIC + VIC_RAWINTR));
  sc_printf("  PIC_INTENABLE = %x\n", *(PIC + VIC_INTENABLE));
}

void set_interrupt_handler(unsigned char irq, isr_t isr) {
  sc_LOGF_IF(TRACE_INTERRUPTS,
    "irq = %x   isr = %x", irq, (unsigned int) isr);

  ASSERT(interrupt_handlers[irq] == NULL);
  interrupt_handlers[irq] = isr;
}

void enable_interrupt(unsigned char irq) {
  // Enable interrupt on controller

  sc_LOGF_IF(TRACE_INTERRUPTS, "irq = %x", irq);

  *(PIC + VIC_INTENABLE) |= (1 << irq);
}
