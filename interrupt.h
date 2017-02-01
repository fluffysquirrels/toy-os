#include "kernel.h"
#include "thread.h"

typedef void (*isr_t)(void);
void set_interrupt_handler(unsigned char irq, isr_t isr);
void enable_interrupt(unsigned char irq);
void handle_interrupt();
unsigned int interrupt_get_status();