#include "scheduler.h"

#include "asm_constants.h"
#include "context_switch.h"
#include "interrupt.h"
#include "stdlib.h"
#include "console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "third_party/OpenBSD_collections/src/queue.h"
#include "thread.h"
#include "timer.h"
#include "util.h"

#ifndef TRACE_SCHEDULER
#define TRACE_SCHEDULER 0
#endif

static void trq_init();
static void trq_remove(unsigned int prio, struct thread_t* thread);
static void trq_append(unsigned int prio, struct thread_t* thread);
static struct thread_t* trq_peek_first(unsigned int prio);

static void set_reschedule_timer(struct thread_t *t);

#define THREAD_QUANTUM_DURATION (DURATION_MS * 500)

void scheduler_init() {
  trq_init();
}

void scheduler_loop() {
  LOG_IF(TRACE_SCHEDULER, "start");

  while (1) {
    LOG_IF(TRACE_SCHEDULER, "top of loop");

    bool interrupt_active = interrupt_get_active() != IRQ_NONE;
    if (interrupt_active) {
      interrupt_handle();
      continue;
    }

    unsigned int prio = THREAD_PRIORITY_MAX;
    struct thread_t *thread = NULL;

    // Loop over priorities from MAX to MIN.
    // Couldn't figure out a way to do this with a for loop given
    // thread priority is unsigned.
    while (1) {
      thread = trq_peek_first(prio);
      if (thread != NULL) {
        break;
      }

      if (prio <= 0) {
        break;
      }
      prio--;
    }

    if (thread == NULL) {
      LOG_IF(TRACE_SCHEDULER, "no threads ready, sleeping");

      set_reschedule_timer(NULL);

      sleep();
      LOG_IF(TRACE_SCHEDULER, "woke up after sleep\n");

      continue;
    }

#if TRACE_SCHEDULER
    LOGF("thread_id=%x", thread->thread_id);
    sc_print_thread(thread);
#endif // TRACE_SCHEDULER

    ASSERT(thread->state == THREAD_STATE_READY);

    set_reschedule_timer(thread);

    unsigned int stop_reason = activate_thread(thread);

    LOGF_IF(TRACE_SCHEDULER,
      "activate_thread returned %x%s\n",
      stop_reason,
      stop_reason == ACTIVATE_RET_IRQ     ? "=ACTIVATE_RET_IRQ"
    : stop_reason == ACTIVATE_RET_SYSCALL ? "=ACTIVATE_RET_SYSCALL": "");

    if (stop_reason == ACTIVATE_RET_IRQ) {
      interrupt_handle(thread);

      // Cycle the thread to the back of the run queue
      trq_remove(thread->priority, thread);
      trq_append(thread->priority, thread);
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      handle_syscall(thread);
    }
  } // scheduler while loop

  /* Not reached */
}

static void set_reschedule_timer(struct thread_t *t) {
  UNUSED(t);

  // TODO: Store how much time a thread has consumed since it was
  //       last scheduled and subtract that from its quantum to calculate how
  //       long to run it.

  duration_t thread_deadline = timer_systemnow() + THREAD_QUANTUM_DURATION;
  duration_t timer_deadline = timer_get_earliest_deadline();
  duration_t min_deadline = MIN(thread_deadline, timer_deadline);

  timer_set_deadline(min_deadline);
}

// trq = thread run queues
struct thread_queue_entry {
  TAILQ_ENTRY(thread_queue_entry) queue_entry;
  struct thread_t *thread;
};

TAILQ_HEAD(thread_queue, thread_queue_entry);

static struct thread_queue thread_run_queues[THREAD_PRIORITY_MAX + 1];

static void trq_init() {
  for (unsigned int prio = 0; prio <= THREAD_PRIORITY_MAX; prio++) {
    TAILQ_INIT(&thread_run_queues[prio]);
  }
}

// TODO: Maybe speed this up by storing a pointer to thread_queue_entry
// in thread_t.
static void trq_remove(unsigned int prio, struct thread_t *thread) {
  struct thread_queue *q = &thread_run_queues[prio];
  {
    struct thread_queue_entry *it, *tmp = NULL;
    TAILQ_FOREACH_SAFE(it, q, queue_entry, tmp) {
      if (it->thread == thread) {
        TAILQ_REMOVE(q, it, queue_entry);
        free(it);
        it = NULL;
        return;
      }
    }
  }

  PANIC("Didn't find the thread.");
}

static void trq_append(unsigned int prio, struct thread_t *thread) {
  struct thread_queue_entry *qe = malloc(sizeof(struct thread_queue_entry));
  ASSERT(qe != NULL);
  qe->thread = thread;
  TAILQ_INSERT_TAIL(&thread_run_queues[prio], qe, queue_entry);
}

static struct thread_t *trq_peek_first(unsigned int prio) {
  struct thread_queue_entry *qe = TAILQ_FIRST(&thread_run_queues[prio]);
  return qe == NULL ? NULL : qe->thread;
}

void scheduler_update_thread_priority(struct thread_t* thread, unsigned int old_priority) {
  if (thread->state == THREAD_STATE_READY) {
    trq_remove(old_priority, thread);
    trq_append(thread->priority, thread);
  }
}

void scheduler_update_thread_state(struct thread_t* thread, unsigned int old_state) {
  if (old_state == THREAD_STATE_READY) {
    trq_remove(thread->priority, thread);
  }

  if (thread->state == THREAD_STATE_READY) {
    trq_append(thread->priority, thread);
  }
}
