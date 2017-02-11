#include "interrupt_raspi.h"

#include "arch_interrupt.h"
#include "arch_interrupt_numbers.h"
#include "arch_registers.h"
#include "interrupt_raspi_reg.h"
#include "stdint.h"
#include "synchronous_console.h"
#include "util.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

volatile uint32_t *base = (volatile uint32_t*) IC_RASPI_BASE;

static void interrupt_raspi_log_status();
static void interrupt_raspi_enable_interrupt(uint8_t irq);
static irq interrupt_raspi_get_active_interrupt();

static void interrupt_raspi_init() {}

static void interrupt_raspi_log_status() {
  sc_LOGF("@ %x", IC_RASPI_BASE);
  sc_print_uint32_mem("  IC_RASPI_PENDING_BASIC  ", base + IC_RASPI_PENDING_BASIC);
  sc_print_uint32_mem("  IC_RASPI_PENDING_1      ", base + IC_RASPI_PENDING_1    );
  sc_print_uint32_mem("  IC_RASPI_PENDING_2      ", base + IC_RASPI_PENDING_2    );
  sc_puts("\n");
  sc_print_uint32_mem("  IC_RASPI_FIQ_CONTROL    ", base + IC_RASPI_FIQ_CONTROL  );
  sc_puts("\n");
  sc_print_uint32_mem("  IC_RASPI_ENABLE_1       ", base + IC_RASPI_ENABLE_1     );
  sc_print_uint32_mem("  IC_RASPI_ENABLE_2       ", base + IC_RASPI_ENABLE_2     );
  sc_print_uint32_mem("  IC_RASPI_ENABLE_BASIC   ", base + IC_RASPI_ENABLE_BASIC );
  sc_puts("\n");
  sc_print_uint32_mem("  IC_RASPI_DISABLE_1      ", base + IC_RASPI_DISABLE_1    );
  sc_print_uint32_mem("  IC_RASPI_DISABLE_2      ", base + IC_RASPI_DISABLE_2    );
  sc_print_uint32_mem("  IC_RASPI_DISABLE_BASIC  ", base + IC_RASPI_DISABLE_BASIC);

  sc_puts("\n");
}

static void interrupt_raspi_enable_interrupt(uint8_t irq) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits" // Ignore 0 <= unsigned
  if (INTNUM_RASPI_BASIC_BANK_MIN <= irq && irq <= INTNUM_RASPI_BASIC_BANK_MAX) {
#pragma GCC diagnostic pop
    *(base + IC_RASPI_ENABLE_BASIC) = 1 << (irq - INTNUM_RASPI_BASIC_BANK_MIN);
  } else if (INTNUM_RASPI_BANK_1_MIN <= irq && irq <= INTNUM_RASPI_BANK_1_MAX) {
    *(base + IC_RASPI_ENABLE_1) = 1 << (irq - INTNUM_RASPI_BANK_1_MIN);
  } else if (INTNUM_RASPI_BANK_2_MIN <= irq && irq <= INTNUM_RASPI_BANK_2_MAX) {
    *(base + IC_RASPI_ENABLE_2) = 1 << (irq - INTNUM_RASPI_BANK_2_MIN);
  } else {
    PANICF("irq %u in unknown bank", (uint32_t) irq);
  }
}

static irq interrupt_raspi_get_active_interrupt() {
  uint32_t basic = *(base + IC_RASPI_PENDING_BASIC);
  // masked_basic ignores bits signifying that interrupts are pending in
  // bank 1 or bank 2.
  uint32_t masked_basic = basic & INTNUM_RASPI_BASIC_BANK_MASK;
  uint32_t bank1 = *(base + IC_RASPI_PENDING_1);
  uint32_t bank2 = *(base + IC_RASPI_PENDING_2);
  if (masked_basic != 0) {
    int bank_irq = __builtin_ctz(basic);
    return INTNUM_RASPI_BASIC_BANK_MIN + bank_irq;
  } else if (bank1 != 0) {
    int bank_irq = __builtin_ctz(bank1);
    return INTNUM_RASPI_BANK_1_MIN + bank_irq;
  } else if (bank2 != 0) {
    int bank_irq = __builtin_ctz(bank2);
    return INTNUM_RASPI_BANK_2_MIN + bank_irq;
  }

  // No banks non-zero
  return IRQ_NONE;
}

static struct interrupt_controller ic = {
  .init = interrupt_raspi_init,
  .log_status = interrupt_raspi_log_status,
  .enable_interrupt = interrupt_raspi_enable_interrupt,
  .get_active_interrupt = interrupt_raspi_get_active_interrupt,
};

struct interrupt_controller* interrupt_raspi_get_ic() {
  return &ic;
}
