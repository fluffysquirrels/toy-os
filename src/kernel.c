#include "kernel.h"

#include "interrupt.h"
#include "scheduler.h"
#include "stdbool.h"
#include "console.h"
#include "syscall_handlers.h"
#include "timer.h"
#include "uart.h"

static bool init_complete = false;

void kernel_init() {
  init_syscall_handlers();
  interrupt_init();
  uart_init();
  timer_init();
  scheduler_init();

  init_complete = true;
}

void kernel_run() {
  ASSERT_MSG(init_complete, "Must call kernel_init() before kernel_run().");
  scheduler_loop();
}
