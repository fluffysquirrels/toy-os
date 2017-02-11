#include "interrupt_pl190.h"

#include "arch_interrupt.h"
#include "interrupt_pl190_reg.h"
#include "stdint.h"
#include "synchronous_console.h"
#include "util.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

static void interrupt_pl190_log_status();
static uint32_t interrupt_pl190_get_status();
static void interrupt_pl190_enable_interrupt(uint8_t irq);
static irq interrupt_pl190_get_active_interrupt();

static uint32_t interrupt_pl190_get_status() {
  return *(PIC + VIC_IRQSTATUS);
}

static void interrupt_pl190_log_status() {
  sc_LOG("");
  sc_print_uint32_memv("  VIC_IRQSTATUS", PIC + VIC_IRQSTATUS);
  sc_print_uint32_memv("  VIC_RAWINTR  ", PIC + VIC_RAWINTR  );
  sc_print_uint32_memv("  VIC_INTENABLE", PIC + VIC_INTENABLE);
}

static void interrupt_pl190_enable_interrupt(uint8_t irq) {
  *(PIC + VIC_INTENABLE) |= (1 << irq);
}

static irq interrupt_pl190_get_active_interrupt() {
  uint32_t pic_irqstatus = interrupt_pl190_get_status();
  if (pic_irqstatus == 0) {
    return IRQ_NONE;
  }
  int irq = __builtin_ctz(pic_irqstatus);
  return irq;
}

static void interrupt_pl190_init() {}

static struct interrupt_controller ic = {
  .init = interrupt_pl190_init,
  .log_status = interrupt_pl190_log_status,
  .enable_interrupt = interrupt_pl190_enable_interrupt,
  .get_active_interrupt = interrupt_pl190_get_active_interrupt,
};

struct interrupt_controller* interrupt_pl190_get_ic() {
  return &ic;
}
