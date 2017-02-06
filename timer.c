// TODO: Find a better name for this higher level OS timer functionality?

#include "timer.h"

#include "rtc_pl031.h"
#include "stdlib.h"
#include "synchronous_console.h"
#include "third_party/OpenBSD_collections/src/tree.h"
#include "timer_sp804.h"
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

// Store the highest systemnow value we've calculated so far.
static time highest_systemnow = 0;

void timer_init() {
  highest_systemnow = 0;
}

void timer_rtc_tick() {
  sc_LOG_IF(TRACE_TIMER, "");
  timer_sp804_set_timeout(TIMER_SP804_1S_COUNTDOWN_TIMER, 1000 * TIMER_SP804_TICKS_PER_MS);
}

time timer_systemnow() {
  time rtc_since_startup = DURATION_S * ((uint64_t) rtc_pl031_get_current());

  // We reset TIMER1 every tick of the RTC (1Hz)and it counts
  // down from 1000000 at 1MHz.
  time hi_res_time_since_rtc_tick = DURATION_S - DURATION_US * ((uint64_t) timer_sp804_get_current(TIMER_SP804_1S_COUNTDOWN_TIMER));
  time now = rtc_since_startup + hi_res_time_since_rtc_tick;
  highest_systemnow = MAX(now, highest_systemnow);

  // Return the highest systemnow value we've calculated so far, so that time
  // doesn't go backwards. This breaks timers, for example.
  return highest_systemnow;
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

  sc_LOGF_IF(TRACE_TIMER, "timer_id = %u", timer_id);

  // Note: We don't need to set a hardware timer. Before it activates a thread
  // the scheduler will set its timer to fire before the earliest due time of
  // any of our timers.

  return E_SUCCESS;
}

void timer_do_expired_callbacks() {
  sc_LOG_IF(TRACE_TIMER, "");

  time now = timer_systemnow();

  while(1) {
    struct timer_node *n = RB_MIN(timer_map, &timers);
    if (n == NULL || n->deadline > now) {
      return;
    }
#if TRACE_TIMER
    sc_LOGF_IF(TRACE_TIMER,
        "Found expired timer id=%u\n"
        "  deadline = %llu\n"
        "  now      = %llu", n->timer_id, n->deadline, now);
    sc_puts("  deadline = ");
    sc_print_uint64_dec(n->deadline);
    sc_puts("\n");
    sc_puts("  now      = ");
    sc_print_uint64_dec(now);
    sc_puts("\n");
#endif

    timer_do_callback(n);
    RB_REMOVE(timer_map, &timers, n);
    free(n);
  }
}

static void timer_do_callback(struct timer_node *n) {
  sc_LOGF_IF(TRACE_TIMER, "timer_id=%u", n->timer_id);

  struct timer_callback_data_t cbd = {
    .timer_id = n->timer_id,
  };
  n->callback(cbd, n->callback_state);
}

time timer_get_earliest_deadline() {
  struct timer_node *n = RB_MIN(timer_map, &timers);
  if (n == NULL) {
    return UINT64_MAX;
  }
  return n->deadline;
}
