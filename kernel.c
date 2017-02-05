#include "kernel.h"

#include "asm_constants.h"
#include "context_switch.h"
#include "interrupt.h"
#include "third_party/OpenBSD_collections/src/queue.h"
#include "rtc_pl031.h"
#include "stdbool.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "thread.h"
#include "timer.h"
#include "timer_sp804.h"
#include "uart.h"
#include "util.h"

#ifndef TRACE_SCHEDULER
#define TRACE_SCHEDULER 0
#endif

static void scheduler_loop(void);

static void scheduler_init();

static void trl_init();
static bool trl_remove(unsigned int prio, struct thread_t* thread);
static void trl_append(unsigned int prio, struct thread_t* thread);
static struct thread_t* trl_peek_first(unsigned int prio);

static bool init_complete = false;

void kernel_init() {
  init_syscall_handlers();
  scheduler_init();

  // Init devices
  uart_init();
  rtc_pl031_init();
  timer_sp804_init();
  // After devices

  timer_init();

  init_complete = true;
}

void kernel_run() {
  assert(init_complete, "didn't call kernel_init() before kernel_run()");
  scheduler_loop();
}

#define THREAD_QUANTUM_DURATION (DURATION_MS * 500)

static void scheduler_loop() {
  sc_LOG_IF(TRACE_SCHEDULER, "start");

  while(1) {
    sc_LOG_IF(TRACE_SCHEDULER, "top of loop");

    bool interrupt_active = interrupt_get_status() != 0;
    if(interrupt_active) {
      handle_interrupt();
      continue;
    }

    unsigned int prio = THREAD_PRIORITY_MAX;
    struct thread_t *thread = NULL;

    // Loop over priorities from MAX to MIN.
    // Couldn't figure out a way to do this with a for loop given
    // thread priority is unsigned.
    while (1) {
      thread = trl_peek_first(prio);
      if (thread != NULL) {
        break;
      }

      if (prio <= 0) {
        break;
      }
      prio--;
    }

    if (thread == NULL) {
      sc_LOG_IF(TRACE_SCHEDULER, "no threads ready, sleeping");

      // TODO: Reduce duplication between this and activate.
      duration_t max_thread_time = THREAD_QUANTUM_DURATION;
      duration_t max_timer_time = timer_get_earliest_deadline() - timer_systemnow();
      duration_t max_time = MIN(max_thread_time, max_timer_time);
      uint64_t max_time_ticks = max_time / (DURATION_MS / TIMER_SP804_TICKS_PER_MS);
      timer_sp804_set_timeout(TIMER_SP804_SCHEDULER_TIMER, (uint32_t) max_time_ticks);

      sleep();
      sc_LOG_IF(TRACE_SCHEDULER, "woke up after sleep\n");

      continue;
    }

#if TRACE_SCHEDULER
    sc_LOGF("thread_id=%x", thread->thread_id);
    sc_print_thread(thread);
#endif // TRACE_SCHEDULER

    ASSERT(thread->state == THREAD_STATE_READY);

    // TODO: Store how much time a thread has consumed since it was
    //       last scheduled and subtract that from its quantum for time to run it.

    // TODO: Reduce duplication between this and no threads to run.
    duration_t max_thread_time = THREAD_QUANTUM_DURATION;
    duration_t max_timer_time = timer_get_earliest_deadline() - timer_systemnow();
    duration_t max_time = MIN(max_thread_time, max_timer_time);
    uint64_t max_time_ticks = max_time / (DURATION_MS / TIMER_SP804_TICKS_PER_MS);
    timer_sp804_set_timeout(TIMER_SP804_SCHEDULER_TIMER, (uint32_t) max_time_ticks);

    unsigned int stop_reason = activate(thread);

    sc_LOGF_IF(TRACE_SCHEDULER,
      "activate returned %x%s\n",
      stop_reason,
      stop_reason == ACTIVATE_RET_IRQ     ? "=ACTIVATE_RET_IRQ"
    : stop_reason == ACTIVATE_RET_SYSCALL ? "=ACTIVATE_RET_SYSCALL": "");

    if(stop_reason == ACTIVATE_RET_IRQ) {
      handle_interrupt(thread);

      // Cycle the thread to the back of the run list
      trl_remove(thread->priority, thread);
      trl_append(thread->priority, thread);
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      handle_syscall(thread);
    }
  } // scheduler while loop

  /* Not reached */
}

static void scheduler_init() {
  trl_init();
}

// trl = thread run lists
// TODO: Better data structure.
struct thread_t* thread_run_lists[THREAD_PRIORITY_MAX + 1][THREAD_LIMIT];



static void trl_init() {
  for (unsigned int prio = 0; prio <= THREAD_PRIORITY_MAX; prio++) {
    for (unsigned int tid = 0; tid < THREAD_LIMIT; tid++) {
      thread_run_lists[prio][tid] = NULL;
    }
  }
}

// Return true if we found thread and removed it
static bool trl_remove(unsigned int prio, struct thread_t *thread) {
  for (unsigned int idx = 0; idx < THREAD_LIMIT; idx++) {
    if (thread_run_lists[prio][idx] == thread) {

      // Shuffle up the remaining items.
      // TODO: Early exit.
      thread_run_lists[prio][idx] = NULL;
      for (unsigned int j = idx; j < THREAD_LIMIT - 1; j++) {
        thread_run_lists[prio][j] = thread_run_lists[prio][j + 1];
      }
      thread_run_lists[prio][THREAD_LIMIT - 1] = NULL;

      return true;
    }
  }

  return false;
}

static void trl_append(unsigned int prio, struct thread_t *thread) {
  for (unsigned int i = 0; i < THREAD_LIMIT; i++) {
    if (thread_run_lists[prio][i] == NULL) {
      thread_run_lists[prio][i] = thread;
      return;
    }
  }

  PANIC("Didn't find a free slot in trl");
}

static struct thread_t *trl_peek_first(unsigned int prio) {
  return thread_run_lists[prio][0];
}

void scheduler_update_thread_priority(struct thread_t* thread, unsigned int old_priority) {
  if (thread->state == THREAD_STATE_READY) {
    trl_remove(old_priority, thread);
    trl_append(thread->priority, thread);
  }
}

void scheduler_update_thread_state(struct thread_t* thread, unsigned int old_state) {
  if (old_state == THREAD_STATE_READY) {
    trl_remove(thread->priority, thread);
  }

  if (thread->state == THREAD_STATE_READY) {
    trl_append(thread->priority, thread);
  }
}
