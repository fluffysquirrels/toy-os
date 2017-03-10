// TODO: Find a better name for this higher level OS timer functionality?

#include "timer.h"

#include "arch_timer.h"
#include "stdlib.h"
#include "console.h"
#include "third_party/OpenBSD_collections/src/tree.h"
#include "util.h"

#ifndef TRACE_TIMER
#define TRACE_TIMER 0
#endif

struct timer_node {
  RB_ENTRY(timer_node) rb;
  time deadline;
  timer_callback_t callback;
  void *callback_state;
  timer_id_t timer_id;
};

static void timer_do_callback(struct timer_node *n);

DEFINE_KEY_COMPARER(timer_node_cmp, struct timer_node, time, deadline)

static RB_HEAD(timer_map, timer_node) timers = RB_INITIALIZER(&timers);
RB_GENERATE(timer_map, timer_node, rb, timer_node_cmp)

static timer_id_t next_timer_id = 0;

static struct arch_timer *at = NULL;

void timer_init() {
  at = arch_get_timer();
  at->init();
}

time timer_systemnow() {
  if (at == NULL) {
    return 0;
  }
  return at->systemnow();
}

err_t timer_queue(
    duration_t d,
    timer_callback_t callback,
    void *callback_state,
    timer_id_t *out_timer_id) {

  struct timer_node *n = malloc(sizeof(struct timer_node));
  ASSERT(n != NULL);
  n->deadline = timer_systemnow() + d;
  n->callback = callback;
  n->callback_state = callback_state;
  timer_id_t timer_id = next_timer_id++;
  n->timer_id = timer_id;
  RB_INSERT(timer_map, &timers, n);

  if (out_timer_id != NULL) {
    *out_timer_id = timer_id;
  }

  LOGF_IF(TRACE_TIMER, "timer_id = %u", timer_id);

  return E_SUCCESS;
}

void timer_set_deadline(time t) {
  ASSERT(at != NULL);
  at->set_deadline(t);
}

void timer_do_expired_callbacks() {
  LOG_IF(TRACE_TIMER, "");

  time now = timer_systemnow();

  while(1) {
    struct timer_node *n = RB_MIN(timer_map, &timers);
    if (n == NULL || n->deadline > now) {
      return;
    }

    LOGF_IF(TRACE_TIMER,
        "Found expired timer id=%u\n"
        "  deadline = %llu\n"
        "  now      = %llu", n->timer_id, n->deadline, now);

    timer_do_callback(n);
    RB_REMOVE(timer_map, &timers, n);
    free(n);
    // timer_set_next_arch_deadline();
  }
}

static void timer_do_callback(struct timer_node *n) {
  LOGF_IF(TRACE_TIMER, "timer_id = %u", n->timer_id);
  LOGF_IF(TRACE_TIMER, "callback = %x", n->callback);

  if (n->callback != NULL) {
    struct timer_callback_data_t cbd = {
      .timer_id = n->timer_id,
    };
    n->callback(cbd, n->callback_state);
  }
}

time timer_get_earliest_deadline() {
  struct timer_node *n = RB_MIN(timer_map, &timers);
  if (n == NULL) {
    return UINT64_MAX;
  }
  return n->deadline;
}

void timer_delay(duration_t d) {
  LOGF_IF(TRACE_TIMER, "d = %llu", d);
  time now = timer_systemnow();
  time deadline = now + d;
  timer_delay_until(deadline);
}

void timer_delay_until(time t) {
  LOGF_IF(TRACE_TIMER, "t   = %llu", t);
  LOGF_IF(TRACE_TIMER, "now = %llu", timer_systemnow());

  while (timer_systemnow() < t) {}
}
