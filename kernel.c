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

#ifndef TRACE_SCHEDULER
#define TRACE_SCHEDULER 0
#endif

static void scheduler_loop(void);

static void init_timers(void);
static void start_scheduler_timer(void);

static void init_scheduler();

static void trl_init();
static bool trl_remove(unsigned int prio, unsigned int thread_id);
static void trl_append(unsigned int prio, unsigned int thread_id);
static unsigned int trl_peek_first(unsigned int prio);

static bool init_complete = false;

void kernel_init() {
  init_syscall_handlers();
  init_scheduler();

  // Init devices
  uart_init();
  init_timers();
  // After devices

  start_scheduler_timer();

  init_complete = true;
}

void kernel_run() {
  assert(init_complete, "didn't call kernel_init() before kernel_run()");
  scheduler_loop();
}

static void scheduler_loop() {
  sc_LOG_IF(TRACE_SCHEDULER, "start");

  while(1) {
    sc_LOG_IF(TRACE_SCHEDULER, "top of loop");

    bool interrupt_active = interrupt_get_status() != 0;
    if(interrupt_active) {
      handle_interrupt();
      continue;
    }

    unsigned int thread_id = THREAD_ID_INVALID;
    unsigned int prio = THREAD_PRIORITY_MAX;

    // Loop over priorities from MAX to MIN.
    // Couldn't figure out a way to do this with a for loop given
    // thread priority is unsigned.
    while (1) {
      unsigned int peek = trl_peek_first(prio);
      if (peek != THREAD_ID_INVALID) {
        thread_id = peek;
        break;
      }

      if (prio <= 0) {
        break;
      }
      prio--;
    }

    if (thread_id == THREAD_ID_INVALID) {
      sc_LOG_IF(TRACE_SCHEDULER, "no threads ready, sleeping\n\n");

      sleep();
      continue;
    }

    struct thread_t *thread = thread_get(thread_id);

#if TRACE_SCHEDULER
    sc_LOGF("thread_id=%x", thread_id);
    sc_print_thread(thread);
#endif // TRACE_SCHEDULER

    ASSERT(thread->state == THREAD_STATE_READY);

    unsigned int stop_reason = activate(thread);

    sc_LOGF_IF(TRACE_SCHEDULER,
      "activate returned %x%s\n",
      stop_reason,
      stop_reason == ACTIVATE_RET_IRQ     ? "=ACTIVATE_RET_IRQ"
    : stop_reason == ACTIVATE_RET_SYSCALL ? "=ACTIVATE_RET_SYSCALL": "");

    if(stop_reason == ACTIVATE_RET_IRQ) {
      handle_interrupt(thread);

      // Cycle the thread to the back of the run list
      trl_remove(thread->priority, thread->thread_id);
      trl_append(thread->priority, thread->thread_id);
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      handle_syscall(thread);
    }
  } // scheduler while loop

  /* Not reached */
}

static void init_scheduler() {
  trl_init();
}

// trl = thread run lists
// TODO: Better data structure.
unsigned int thread_run_lists[THREAD_PRIORITY_MAX + 1][THREAD_LIMIT];

static void trl_init() {
  for (unsigned int prio = 0; prio <= THREAD_PRIORITY_MAX; prio++) {
    for (unsigned int tid = 0; tid < THREAD_LIMIT; tid++) {
      thread_run_lists[prio][tid] = THREAD_ID_INVALID;
    }
  }
}

// Return true if we found thread_id and removed it
static bool trl_remove(unsigned int prio, unsigned int thread_id) {
  for (unsigned int idx = 0; idx < THREAD_LIMIT; idx++) {
    if (thread_run_lists[prio][idx] == thread_id) {

      // Shuffle up the remaining items.
      // TODO: Early exit.
      thread_run_lists[prio][idx] = THREAD_ID_INVALID;
      for (unsigned int j = idx; j < THREAD_LIMIT - 1; j++) {
        thread_run_lists[prio][j] = thread_run_lists[prio][j + 1];
      }
      thread_run_lists[prio][THREAD_LIMIT - 1] = THREAD_ID_INVALID;

      return true;
    }
  }

  return false;
}

static void trl_append(unsigned int prio, unsigned int thread_id) {
  for (unsigned int i = 0; i < THREAD_LIMIT; i++) {
    if (thread_run_lists[prio][i] == THREAD_ID_INVALID) {
      thread_run_lists[prio][i] = thread_id;
      return;
    }
  }

  PANIC("Didn't find a free slot in trl");
}

static unsigned int trl_peek_first(unsigned int prio) {
  return thread_run_lists[prio][0];
}

void scheduler_update_thread_priority(struct thread_t* thread, unsigned int old_priority) {
  if (thread->state == THREAD_STATE_READY) {
    unsigned int tid = thread->thread_id;
    trl_remove(old_priority, tid);
    trl_append(thread->priority, tid);
  }
}

void scheduler_update_thread_state(struct thread_t* thread, unsigned int old_state) {
  unsigned int tid = thread->thread_id;
  if (old_state == THREAD_STATE_READY) {
    trl_remove(thread->priority, tid);
  }

  if (thread->state == THREAD_STATE_READY) {
    trl_append(thread->priority, tid);
  }
}

static void isr_timer01() {
  sc_LOG_IF(TRACE_SCHEDULER, "start");

  if(*(TIMER0 + TIMER_MIS)) { /* Timer0 went off */
    *(TIMER0 + TIMER_INTCLR) = 1; /* Clear interrupt */
    sc_LOG_IF(TRACE_SCHEDULER, "TIMER0 tick");
  } else if(*(TIMER1 + TIMER_MIS)) { /* Timer1 went off */
    *(TIMER1 + TIMER_INTCLR) = 1; /* Clear interrupt */
    sc_LOG_IF(TRACE_SCHEDULER, "TIMER1 tick");
  } else {
    PANIC("*(TIMER0/1 + TIMER_MIS) was clear");
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
