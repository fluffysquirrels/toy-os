#pragma once

#include "kernel.h"
#include "thread.h"

// TODO: Move these somewhere else.
// Interrupt numbers
/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/Chdbeibh.html */
#define PIC_INTNUM_COUNT    32
#define PIC_INTNUM_TIMER01  4 // Interrupt for timers 0 and 1
#define PIC_INTNUM_TIMER23  5 // Interrupt for timers 2 and 3
#define PIC_INTNUM_RTC      10
#define PIC_INTNUM_UART0    12
#define PIC_INTNUM_UART1    13
#define PIC_INTNUM_UART2    14
#define PIC_INTNUM_DMA      17

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
