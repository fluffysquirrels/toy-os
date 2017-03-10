#include "timer_versatilepb.h"

#include "arch_timer.h"
#include "rtc_pl031.h"
#include "stdint.h"
#include "console.h"
#include "syscalls.h"
#include "timer.h"
#include "timer_sp804.h"
#include "util.h"

#ifndef TRACE_TIMER
#define TRACE_TIMER 0
#endif

// Store the highest systemnow value we've calculated so far.
static time highest_systemnow = 0;

void timer_versatilepb_init() {
  highest_systemnow = 0;
  rtc_pl031_init();
  timer_sp804_init();
}

void timer_versatilepb_rtc_tick() {
  LOG_IF(TRACE_TIMER, "");
  timer_sp804_set_timeout(TIMER_SP804_1S_COUNTDOWN_TIMER, 1000 * TIMER_SP804_TICKS_PER_MS);
}

time timer_versatilepb_systemnow() {
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

void timer_versatilepb_set_timeout(duration_t d) {
  uint64_t d_ticks = d / (DURATION_MS / TIMER_SP804_TICKS_PER_MS);
  uint32_t d_ticks_uint32 =
      d_ticks > UINT32_MAX
    ? UINT32_MAX // Wake up after UINT32_MAX on overflow
                 // We can go back to sleep again then.
    : (uint32_t) d_ticks;
  timer_sp804_set_timeout(TIMER_SP804_TIMEOUT_TIMER, d_ticks_uint32);
}

void timer_versatilepb_set_deadline(time t) {
  time now = timer_systemnow();
  duration_t timeout = t < now ? 0 : t - now;

  timer_versatilepb_set_timeout(timeout);
}

static struct arch_timer at = {
  .init = timer_versatilepb_init,
  .systemnow = timer_versatilepb_systemnow,
  .set_deadline = timer_versatilepb_set_deadline,
  .set_timeout = timer_versatilepb_set_timeout,
};

struct arch_timer *timer_versatilepb_get_arch_timer() {
  return &at;
}
