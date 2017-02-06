#include "interrupt.h"
#include "rtc_pl031.h"
#include "rtc_pl031_reg.h"
#include "synchronous_console.h"
#include "timer.h"
#include "timer_versatilepb.h"
#include "util.h"

#ifndef TRACE_PL031
#define TRACE_PL031 0
#endif

static void rtc_interrupt();
static void rtc_set_match_in_future();

#if TRACE_PL031
static void rtc_log_state();
#endif

// Store the highest time value we've observed.
static uint32_t highest_value = 0;

static uint32_t rtc_pl031_get_raw() {
  return *(RTC_BASE + RTC_DR);
}

uint32_t rtc_pl031_get_current() {
  uint32_t raw = rtc_pl031_get_raw();
  highest_value = MAX(raw, highest_value);

  // Return the highest time value we've observed. Time should not go backwards.
  return highest_value;
}

void rtc_pl031_init() {
  highest_value = 0;
  interrupt_set_handler(PIC_INTNUM_RTC, &rtc_interrupt);
  interrupt_enable(PIC_INTNUM_RTC);

  // Set counter value to 0
  *(RTC_BASE + RTC_LR) = 0;

  // Start timer
  *(RTC_BASE + RTC_CR) = RTC_CR_START;

  // Enable interrupt
  *(RTC_BASE + RTC_IMSC) = RTC_MIS_IMSC;

  rtc_set_match_in_future();

#if TRACE_PL031
  rtc_log_state();
#endif

}

static void rtc_interrupt() {
  sc_LOG_IF(TRACE_PL031, "");
#if TRACE_PL031
  rtc_log_state();
#endif
  // Clear interrupt
  *(RTC_BASE + RTC_ICR) = RTC_ICR_CLEAR;

  rtc_set_match_in_future();

  // In qemu, this interrupt fires and then some time later (sometimes 500ms)
  // the data register DR gets incremented. However the 1s high resolution timer
  // is reset on this interupt via timer_rtc_tick(), causing systemnow() =
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
  ASSERT(highest_value <= raw);
  highest_value = raw + 1;

#if TRACE_PL031
  rtc_log_state();
#endif

}

static void rtc_set_match_in_future() {
  sc_LOG_IF(TRACE_PL031, "");
  // Set match value to match on next second
  *(RTC_BASE + RTC_MR) = rtc_pl031_get_raw() + 1;
}

#if TRACE_PL031
static void rtc_log_state() {
  sc_LOG("");
  sc_printf("  DR      = %x\n", *(RTC_BASE + RTC_DR));
  sc_printf("  MR      = %x\n", *(RTC_BASE + RTC_MR));
  sc_printf("  LR      = %x\n", *(RTC_BASE + RTC_LR));
  sc_printf("  CR      = %x\n", *(RTC_BASE + RTC_CR));
  sc_printf("  IMSC    = %x\n", *(RTC_BASE + RTC_IMSC));
  sc_printf("  RIS     = %x\n", *(RTC_BASE + RTC_RIS));
  sc_printf("  MIS     = %x\n", *(RTC_BASE + RTC_MIS));
  sc_puts("\n");
  sc_printf("  highest_value  = %u\n", highest_value);
  sc_printf("  raw            = %u\n", *(RTC_BASE + RTC_DR));
}
#endif
