#include "interrupt_pl190.h"

#include "interrupt_pl190_reg.h"
#include "synchronous_console.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

uint32_t interrupt_pl190_get_status() {
  return *(PIC + VIC_IRQSTATUS);
}

void interrupt_pl190_log_status() {
  sc_LOG("");
  sc_printf("  PIC_IRQSTATUS = %x\n", *(PIC + VIC_IRQSTATUS));
  sc_printf("  PIC_RAWINTR   = %x\n", *(PIC + VIC_RAWINTR));
  sc_printf("  PIC_INTENABLE = %x\n", *(PIC + VIC_INTENABLE));
}

void interrupt_pl190_enable(uint8_t irq) {
  *(PIC + VIC_INTENABLE) |= (1 << irq);
}

irq interrupt_pl190_get_active() {
  unsigned int pic_irqstatus = interrupt_pl190_get_status();
  if (pic_irqstatus == 0) {
    return IRQ_NONE;
  }
  int irq = __builtin_ctz(pic_irqstatus);
  return irq;
}
