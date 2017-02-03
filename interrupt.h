#include "kernel.h"
#include "thread.h"

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
void set_interrupt_handler(unsigned char irq, isr_t isr);
void enable_interrupt(unsigned char irq);
void handle_interrupt();
unsigned int interrupt_get_status();
