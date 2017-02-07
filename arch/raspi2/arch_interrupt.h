#pragma once

#include "arch_interrupt_common.h"
#include "interrupt_raspi.h"

struct interrupt_controller* arch_get_interrupt_controller() {
  return interrupt_raspi_get_ic();
}
