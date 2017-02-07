#include "interrupt_raspi.h"

#include "arch_registers.h"
#include "interrupt_raspi_reg.h"
#include "stdint.h"
#include "synchronous_console.h"
#include "util.h"

#ifndef TRACE_INTERRUPTS
#define TRACE_INTERRUPTS 0
#endif

volatile uint32_t *base = (volatile uint32_t*) IC_RPI_BASE;

static void interrupt_raspi_log_status();
static void interrupt_raspi_enable_interrupt(uint8_t irq);
static irq interrupt_raspi_get_active_interrupt();

static void interrupt_raspi_init() {}

static void interrupt_raspi_log_status() {
  sc_LOGF("@ ", IC_RPI_BASE);
  sc_printf( "  active IRQ = %u\n", interrupt_raspi_get_active_interrupt());
  sc_print_uint32_memv("  IC_RPI_PENDING_BASIC  ", base + IC_RPI_PENDING_BASIC);
  sc_print_uint32_memv("  IC_RPI_PENDING_1      ", base + IC_RPI_PENDING_1    );
  sc_print_uint32_memv("  IC_RPI_PENDING_2      ", base + IC_RPI_PENDING_2    );
  sc_puts("\n");
  sc_print_uint32_memv("  IC_RPI_FIQ_CONTROL    ", base + IC_RPI_FIQ_CONTROL  );
  sc_puts("\n");
  sc_print_uint32_memv("  IC_RPI_ENABLE_1       ", base + IC_RPI_ENABLE_1     );
  sc_print_uint32_memv("  IC_RPI_ENABLE_2       ", base + IC_RPI_ENABLE_2     );
  sc_print_uint32_memv("  IC_RPI_ENABLE_BASIC   ", base + IC_RPI_ENABLE_BASIC );
  sc_puts("\n");
  sc_print_uint32_memv("  IC_RPI_DISABLE_1      ", base + IC_RPI_DISABLE_1    );
  sc_print_uint32_memv("  IC_RPI_DISABLE_2      ", base + IC_RPI_DISABLE_2    );
  sc_print_uint32_memv("  IC_RPI_DISABLE_BASIC  ", base + IC_RPI_DISABLE_BASIC);

  sc_puts("\n");
}

// Just support the basic interrupt register for now
static void interrupt_raspi_enable_interrupt(uint8_t irq) {
  ASSERT(irq <= 20);

#if TRACE_INTERRUPTS
  interrupt_raspi_log_status();
  sc_LOGF_IF(TRACE_INTERRUPTS, "irq = %u", irq);
#endif // TRACE_INTERRUPTS

  *(base + IC_RPI_ENABLE_BASIC) = 1 << irq;

#if TRACE_INTERRUPTS
  interrupt_raspi_log_status();
#endif // TRACE_INTERRUPTS
}

static irq interrupt_raspi_get_active_interrupt() {
  unsigned int irqstatus = *(base + IC_RPI_PENDING_BASIC);
  if (irqstatus == 0) {
    return IRQ_NONE;
  }
  int irq = __builtin_ctz(irqstatus);
  return irq;
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
