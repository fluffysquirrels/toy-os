#include "interrupt.h"
#include "rtc_pl031.h"
#include "rtc_pl031_reg.h"
#include "synchronous_console.h"
#include "timer.h"

#ifndef TRACE_PL031
#define TRACE_PL031 0
#endif

static void rtc_interrupt();
static void rtc_set_match_in_future();

#if TRACE_PL031
static void rtc_log_registers();
#endif

uint32_t rtc_pl031_get_current() {
  return *(RTC_BASE + RTC_DR);
}

void rtc_pl031_init() {
  set_interrupt_handler(PIC_INTNUM_RTC, &rtc_interrupt);
  enable_interrupt(PIC_INTNUM_RTC);

  // Set counter value to 0
  *(RTC_BASE + RTC_LR) = 0;

  // Start timer
  *(RTC_BASE + RTC_CR) = RTC_CR_START;

  // Enable interrupt
  *(RTC_BASE + RTC_IMSC) = RTC_MIS_IMSC;

  rtc_set_match_in_future();

#if TRACE_PL031
  rtc_log_registers();
#endif

}

static void rtc_interrupt() {
  sc_LOG_IF(TRACE_PL031, "");
#if TRACE_PL031
  rtc_log_registers();
#endif

  // Clear interrupt
  *(RTC_BASE + RTC_ICR) = RTC_ICR_CLEAR;

  rtc_set_match_in_future();
#if TRACE_PL031
  rtc_log_registers();
#endif

  timer_rtc_tick();
}

static void rtc_set_match_in_future() {
  sc_LOG_IF(TRACE_PL031, "");
  // Set match value to match on next second
  *(RTC_BASE + RTC_MR) = rtc_pl031_get_current() + 1;
}

#if TRACE_PL031
static void rtc_log_registers() {
  sc_LOG("");
  sc_printf("  DR   = %x\n", *(RTC_BASE + RTC_DR));
  sc_printf("  MR   = %x\n", *(RTC_BASE + RTC_MR));
  sc_printf("  LR   = %x\n", *(RTC_BASE + RTC_LR));
  sc_printf("  CR   = %x\n", *(RTC_BASE + RTC_CR));
  sc_printf("  IMSC = %x\n", *(RTC_BASE + RTC_IMSC));
  sc_printf("  RIS  = %x\n", *(RTC_BASE + RTC_RIS));
  sc_printf("  MIS  = %x\n", *(RTC_BASE + RTC_MIS));
}
#endif
