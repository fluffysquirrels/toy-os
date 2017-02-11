#include "timer_raspi.h"

#include "arch_interrupt_numbers.h"
#include "arch_registers.h"
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

static volatile unsigned int* base = (volatile unsigned int*) TIMER_RASPI_BASE;

static void timer_raspi_clear_match();

#ifdef INTNUM_RASPI_TIMER
static void timer_raspi_isr();
static bool timer_raspi_is_match();
#endif // INTNUM_RASPI_TIMER

void timer_raspi_init() {
#ifdef INTNUM_RASPI_TIMER
  interrupt_set_handler(INTNUM_RASPI_TIMER, timer_raspi_isr);
  interrupt_enable(INTNUM_RASPI_TIMER);
#endif // INTNUM_RASPI_TIMER

  timer_sp804_init();
}

#ifdef INTNUM_RASPI_TIMER
static void timer_raspi_isr() {
  sc_LOG_IF(TRACE_TIMER, "");
#if TRACE_TIMER
  timer_raspi_print_status();
#endif
  if (timer_raspi_is_match()) {
    sc_LOG_IF(TRACE_TIMER, "Clearing set match");
    timer_raspi_clear_match();
  } else {
    PANIC("Interrupt was not our match.");
  }

#if TRACE_TIMER
  timer_raspi_print_status();
#endif

  timer_do_expired_callbacks();
}
#endif // INTNUM_RASPI_TIMER

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
  sc_LOGF_IF(TRACE_TIMER, "t = %llu", t);
  uint32_t deadline_count = (uint32_t) (t / DURATION_US);

#if TRACE_TIMER
  timer_raspi_print_status();
#endif

  *(base + TIMER_RASPI_C3) = deadline_count;
#if TRACE_TIMER
  sc_LOG("deadline set");
  timer_raspi_print_status();
#endif

  if (t < timer_raspi_systemnow()) {
    // Deadline has already passed. Handle it directly in case we don't
    // hit an interrupt.

    sc_LOG_IF(TRACE_TIMER, "Deadline already expired. Handling it directly.");
    timer_raspi_clear_match();
    timer_do_expired_callbacks();
  }
}

static void timer_raspi_clear_match() {
  *(base + TIMER_RASPI_CS) = TIMER_RASPI_CS_M3;
}

#ifdef INTNUM_RASPI_TIMER
static bool timer_raspi_is_match() {
  return *(base + TIMER_RASPI_CS) & TIMER_RASPI_CS_M3;
}
#endif // INTNUM_RASPI_TIMER

void timer_raspi_set_timeout(duration_t d) {
  timer_raspi_set_deadline(timer_raspi_systemnow() + d);
}

void timer_raspi_print_status() {
    sc_LOG("");
    sc_print_uint32_memv("  CS ", base + TIMER_RASPI_CS );
    //Documentation says to always read CLO first.
    sc_print_uint32_memv("  CLO", base + TIMER_RASPI_CLO);
    sc_print_uint32_memv("  CHI", base + TIMER_RASPI_CHI);
    sc_print_uint32_memv("  C0 ", base + TIMER_RASPI_C0 );
    sc_print_uint32_memv("  C1 ", base + TIMER_RASPI_C1 );
    sc_print_uint32_memv("  C2 ", base + TIMER_RASPI_C2 );
    sc_print_uint32_memv("  C3 ", base + TIMER_RASPI_C3 );
    sc_printf("  counter  = %llu\n", timer_raspi_get_counter());
    sc_printf("  systemnow = %llu\n", timer_raspi_systemnow());
    sc_printf("  systemnow ms = %llu\n", (timer_raspi_systemnow() / DURATION_MS));
}
