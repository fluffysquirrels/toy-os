#pragma once

#include "interrupt.h"
#include "stdint.h"

void interrupt_pl190_log_status();
uint32_t interrupt_pl190_get_status();
void interrupt_pl190_enable(uint8_t irq);

// Returns the number of an active interrupt, or IRQ_NONE if none are active.
irq interrupt_pl190_get_active();
