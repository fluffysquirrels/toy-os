#include "rtc_pl031.h"

#include "arch_interrupt_numbers.h"
#include "interrupt.h"
#include "rtc_pl031_reg.h"
#include "console.h"
#include "timer.h"
#include "timer_versatilepb.h"
#include "util.h"

#ifndef TRACE_TIMER
#define TRACE_TIMER 0
#endif

static void rtc_interrupt();

static void rtc_set_match_in_future();

// Store the highest time value we've observed.
static uint32_t highest_value = 0;

static uint32_t rtc_pl031_get_raw() {
  return *(RTC_BASE + RTC_DR);
}

uint32_t rtc_pl031_get_current() {
  uint32_t raw = rtc_pl031_get_raw();

#if TRACE_TIMER >= 2
  printf("rtc_pl031_get_current:\n"
         "  raw = %lu\n"
         "  highest_value = %lu\n",
         raw,
         highest_value);
#endif // TRACE_pl031

  highest_value = MAX(raw, highest_value);

  // Return the highest time value we've observed. Time should not go backwards.
  return highest_value;
}

void rtc_pl031_init() {
  LOG_IF(TRACE_TIMER, "");

  interrupt_set_handler(INTNUM_RTC, &rtc_interrupt);
  interrupt_enable(INTNUM_RTC);

  // Start timer
  *(RTC_BASE + RTC_CR) = RTC_CR_START;

  // Enable interrupt
  *(RTC_BASE + RTC_IMSC) = RTC_MIS_IMSC;

  rtc_set_match_in_future();

#if TRACE_TIMER
  rtc_pl031_log_state();
#endif

}
static void rtc_interrupt() {
  LOG_IF(TRACE_TIMER, "");
#if TRACE_TIMER
  rtc_pl031_log_state();
#endif
  // Clear interrupt
  *(RTC_BASE + RTC_ICR) = RTC_ICR_CLEAR;

  rtc_set_match_in_future();

  // In qemu, this interrupt fires and then some time later (sometimes 500ms)
  // the data register DR gets incremented. However the 1s high resolution timer
  // is reset on this interrupt via timer_rtc_tick(), causing systemnow() =
  // rtc seconds + the 1s high resolution timer to jump back 1s.
  // The current hacky fix is to set the high resolution timer back 1s and then
  // immediately increment highest_value by 1, which the raw register should do anyway.
  // This ensures we only go back in time for an instant and code running after
  // this interrupt shouldn't see the blip.
  // Further timer_systemnow() never goes backwards because it saves the latest value
  // it has emitted; this hopefully covers the glitch entirely.

  timer_versatilepb_rtc_tick();

  uint32_t raw = rtc_pl031_get_raw();
  // Assert highest_value is equal to the current raw value (hasn't already been
  // incremented) or is lower, e.g. if the debugger has been running.
  ASSERT_INT_BINOP(highest_value, <=, raw);
  highest_value = raw + 1;

#if TRACE_TIMER
  rtc_pl031_log_state();
#endif
}


static void rtc_set_match_in_future() {
  LOG_IF(TRACE_TIMER, "");
  // Set match value to match on next second
  *(RTC_BASE + RTC_MR) = rtc_pl031_get_raw() + 1;
}

void rtc_pl031_log_state() {
  LOG("");
  sc_print_uint32_mem("  DR  ", (RTC_BASE + RTC_DR));
  sc_print_uint32_mem("  MR  ", (RTC_BASE + RTC_MR));
  sc_print_uint32_mem("  LR  ", (RTC_BASE + RTC_LR));
  sc_print_uint32_mem("  CR  ", (RTC_BASE + RTC_CR));
  sc_print_uint32_mem("  IMSC", (RTC_BASE + RTC_IMSC));
  sc_print_uint32_mem("  RIS ", (RTC_BASE + RTC_RIS));
  sc_print_uint32_mem("  MIS ", (RTC_BASE + RTC_MIS));
  puts("\n");
  printf("  highest_value     = %lu\n", highest_value);
  printf("  raw               = %lu\n", *(RTC_BASE + RTC_DR));
}
