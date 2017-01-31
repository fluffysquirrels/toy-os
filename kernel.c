#include "kernel.h"

#include "asm_constants.h"
#include "context_switch.h"
#include "interrupt.h"
#include "stdbool.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "thread.h"
#include "uart.h"
#include "util.h"
#include "versatilepb.h"

static void kernel_init(void);
static void scheduler_loop(void);

static void init_timers(void);
static void start_scheduler_timer(void);

static void inc_thread_idx(unsigned int *thread_idx);

#ifndef TRACE_SCHEDULER
#define TRACE_SCHEDULER 0
#endif

extern struct thread_t threads[THREAD_LIMIT];
extern unsigned int num_threads;

void kernel_run() {
  kernel_init();
  scheduler_loop();
}

static void kernel_init() {
  init_syscall_handlers();
  uart_init();
  init_timers();
  start_scheduler_timer();
}

static void scheduler_loop() {
#if TRACE_SCHEDULER
    sc_puts("\nscheduler_loop() start\n");
#endif // TRACE_SCHEDULER

  unsigned int thread_idx = 0;

  while(1) {
#if TRACE_SCHEDULER
      sc_puts("\nscheduler_loop()\n");
#endif // TRACE_SCHEDULER

    unsigned int pic_irqstatus = interrupt_get_status();
    bool interrupt_active = pic_irqstatus != 0;
    if(interrupt_active) {
      handle_interrupt(NULL);
      continue;
    }

    if (no_threads_ready()) {
#if TRACE_SCHEDULER
      sc_puts("scheduler_loop() no threads ready, sleeping\n\n");
#endif // TRACE_SCHEDULER

      sleep();
      continue;
    }

    struct thread_t *thread = &threads[thread_idx];

#if TRACE_SCHEDULER
    sc_puts("scheduler_loop() thread_idx=");
    sc_print_uint32_hex(thread_idx);
    sc_puts("\n");
    sc_print_thread(thread);
#endif // TRACE_SCHEDULER

    if (thread->state != THREAD_STATE_READY) {
#if TRACE_SCHEDULER
      sc_puts("scheduler_loop() skipping thread that is not ready\n");
#endif // TRACE_SCHEDULER

      inc_thread_idx(&thread_idx);
      continue;
    }

    unsigned int stop_reason = activate(thread);

#if TRACE_SCHEDULER
    sc_puts("scheduler_loop() activate returned ");
    sc_print_uint32_hex(stop_reason);
    if(stop_reason == ACTIVATE_RET_IRQ) {
      sc_puts(" = ACTIVATE_RET_IRQ");
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      sc_puts(" = ACTIVATE_RET_SYSCALL");
    }
    sc_puts("\n");
#endif // TRACE_SCHEDULER

    if(stop_reason == ACTIVATE_RET_IRQ) {
      handle_interrupt(thread);
      inc_thread_idx(&thread_idx);
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      handle_syscall(thread);
    }
  } // scheduler while loop

  /* Not reached */
}

static void inc_thread_idx(unsigned int *thread_idx) {
   /* Can't use % to calculate new thread_idx because that requires a
      runtime library function */
  (*thread_idx)++;
  if(*thread_idx >= num_threads) {
    *thread_idx = 0;
  }
}

static void isr_timer01() {
#if TRACE_SCHEDULER
    sc_puts("isr_timer01()\n");
#endif // TRACE_SCHEDULER

  if(*(TIMER0 + TIMER_MIS)) { /* Timer0 went off */
    *(TIMER0 + TIMER_INTCLR) = 1; /* Clear interrupt */
#if TRACE_SCHEDULER
    sc_puts("isr_timer01() TIMER0 tick\n");
#endif // TRACE_SCHEDULER
  }  else if(*(TIMER1 + TIMER_MIS)) { /* Timer1 went off */
    *(TIMER1 + TIMER_INTCLR) = 1; /* Clear interrupt */
#if TRACE_SCHEDULER
    sc_puts("isr_timer01() TIMER1 tick\n");
#endif // TRACE_SCHEDULER
  } else {
    panic("isr_timer01() *(TIMER0/1 + TIMER_MIS) was clear");
  }
}

static void init_timers() {
  set_interrupt_handler(PIC_INTNUM_TIMER01, isr_timer01);
  enable_interrupt(PIC_INTNUM_TIMER01);
}

static void start_scheduler_timer() {
  int tickMs = 500;
  *(TIMER0 + TIMER_LOAD) = tickMs * 1000;
  *(TIMER0 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
  *(TIMER0 + TIMER_BGLOAD) = tickMs * 1000;
}
