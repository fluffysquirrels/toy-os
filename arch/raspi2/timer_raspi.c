#include "timer_raspi.h"

#include "arch_interrupt_numbers.h"
#include "arch_registers.h"
#include "arch_timer.h"
#include "context_switch.h"
#include "interrupt.h"
#include "stdint.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "timer.h"
#include "timer_raspi_reg.h"
#include "timer_sp804.h"
#include "util.h"

#ifndef TRACE_TIMER
#define TRACE_TIMER 0
#endif

static volatile uint32_t* base = (volatile uint32_t*) TIMER_RASPI_BASE;

static void timer_raspi_clear_match();

static void timer_raspi_isr();
static bool timer_raspi_is_match();

void timer_raspi_init() {
  interrupt_set_handler(INTNUM_RASPI_TIMER, timer_raspi_isr);
  interrupt_enable(INTNUM_RASPI_TIMER);

  timer_sp804_init();
}

static void timer_raspi_isr() {
  LOG_IF(TRACE_TIMER, "");
#if TRACE_TIMER
  timer_raspi_print_status();
#endif
  if (timer_raspi_is_match()) {
    LOG_IF(TRACE_TIMER, "Clearing set match");
    timer_raspi_clear_match();
  } else {
    PANIC("Interrupt was not our match.");
  }

#if TRACE_TIMER
  timer_raspi_print_status();
#endif

  timer_do_expired_callbacks();
}

time timer_raspi_systemnow() {
  return timer_raspi_get_counter() * DURATION_US;
}

uint64_t timer_raspi_get_counter() {
  // Documentation says to always read CLO first.
  uint32_t lo = *(base + TIMER_RASPI_CLO);
  uint32_t hi = *(base + TIMER_RASPI_CHI);
  return (((uint64_t) hi) << 32) | ((uint64_t) lo);
}

void timer_raspi_set_deadline(time t) {
  LOGF_IF(TRACE_TIMER, "t = %llu", t);
  uint32_t deadline_count = (uint32_t) (t / DURATION_US);

#if TRACE_TIMER
  timer_raspi_print_status();
#endif

  *(base + TIMER_RASPI_C3) = deadline_count;
#if TRACE_TIMER
  LOG("deadline set");
  timer_raspi_print_status();
#endif

  if (t < timer_raspi_systemnow()) {
    // Deadline has already passed. Handle it directly in case we don't
    // hit an interrupt.

    LOG_IF(TRACE_TIMER, "Deadline already expired. Handling it directly.");
    timer_raspi_clear_match();
    timer_do_expired_callbacks();
  }
}

static void timer_raspi_clear_match() {
  *(base + TIMER_RASPI_CS) = TIMER_RASPI_CS_M3;
}

static bool timer_raspi_is_match() {
  return *(base + TIMER_RASPI_CS) & TIMER_RASPI_CS_M3;
}

void timer_raspi_set_timeout(duration_t d) {
  timer_raspi_set_deadline(timer_raspi_systemnow() + d);
}

void timer_raspi_print_status() {
    LOG("");
    sc_print_uint32_mem("  CS ", base + TIMER_RASPI_CS );
    //Documentation says to always read CLO first.
    sc_print_uint32_mem("  CLO", base + TIMER_RASPI_CLO);
    sc_print_uint32_mem("  CHI", base + TIMER_RASPI_CHI);
    sc_print_uint32_mem("  C0 ", base + TIMER_RASPI_C0 );
    sc_print_uint32_mem("  C1 ", base + TIMER_RASPI_C1 );
    sc_print_uint32_mem("  C2 ", base + TIMER_RASPI_C2 );
    sc_print_uint32_mem("  C3 ", base + TIMER_RASPI_C3 );
    printf("  counter  = %llu\n", timer_raspi_get_counter());
    printf("  systemnow = %llu\n", timer_raspi_systemnow());
    printf("  systemnow ms = %llu\n", (timer_raspi_systemnow() / DURATION_MS));
}

static struct arch_timer at = {
  .init = timer_raspi_init,
  .systemnow = timer_raspi_systemnow,
  .set_deadline = timer_raspi_set_deadline,
  .set_timeout = timer_raspi_set_timeout,
};

struct arch_timer *timer_raspi_get_arch_timer() {
  return &at;
}
