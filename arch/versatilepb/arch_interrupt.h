#pragma once

#include "arch_interrupt_common.h"
#include "interrupt_pl190.h"
#include "util.h"

struct interrupt_controller* arch_get_interrupt_controller() {
  return interrupt_pl190_get_ic();
}
