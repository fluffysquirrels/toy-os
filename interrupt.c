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

#if TRACE_INTERRUPTS
  sc_puts("\nhandle_interrupt()\n");
#endif // TRACE_SCHEDULER

  unsigned int pic_irqstatus = interrupt_get_status();

#if TRACE_INTERRUPTS
  pic_log_status();
#endif // TRACE_INTERRUPTS

  if(pic_irqstatus == 0) {
    panic("handle_interrupt() no interrupt active?");
    return;
  }

  int irq = __builtin_ctz(pic_irqstatus);

#if TRACE_INTERRUPTS
  sc_puts("handle_interrupt() irq = ");
  sc_print_uint32_hex(irq);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  assert(irq >= 0, "assert failed: irq >= 0");
  assert(irq < PIC_INTNUM_COUNT, "assert failed: irq <= PIC_INTNUM_COUNT");

  isr_t isr = interrupt_handlers[irq];

  if(!isr) {
    sc_puts("irq = ");
    sc_print_uint32_hex(irq);
    sc_puts("\n");
    panic("Interrupt with no handler");
    // Not reached.
    return;
  }

#if TRACE_INTERRUPTS
  sc_puts("handle_interrupt() calling handler @");
  sc_print_uint32_hex((unsigned int) isr);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  isr();

#if TRACE_INTERRUPTS
  sc_puts("handle_interrupt() returned from handler\n");
  pic_log_status();
#endif // TRACE_INTERRUPTS
}

void pic_log_status() {
  sc_puts("pic_log_status()\n");
  sc_puts("  PIC_IRQSTATUS = ");
  sc_print_uint32_hex(*(PIC + VIC_IRQSTATUS));
  sc_puts("\n");
  sc_puts("  PIC_RAWINTR = ");
  sc_print_uint32_hex(*(PIC + VIC_RAWINTR));
  sc_puts("\n");
  sc_puts("  PIC_INTENABLE = ");
  sc_print_uint32_hex(*(PIC + VIC_INTENABLE));
  sc_puts("\n");
}

void set_interrupt_handler(unsigned char irq, isr_t isr) {
#if TRACE_INTERRUPTS
  sc_puts("set_interrupt_handler() irq = ");
  sc_print_uint32_hex(irq);
  sc_puts(" isr @ ");
  sc_print_uint32_hex((unsigned int) isr);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  assert(interrupt_handlers[irq] == NULL, "failed assert interrupt_handlers[irq] == NULL");
  interrupt_handlers[irq] = isr;
}

void enable_interrupt(unsigned char irq) {
  // Enable interrupt on controller

#if TRACE_INTERRUPTS
  sc_puts("enable_interrupt() irq = ");
  sc_print_uint32_hex(irq);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  *(PIC + VIC_INTENABLE) |= (1 << irq);
}
