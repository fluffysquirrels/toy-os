#include "timer_raspi.h"

#include "arch_registers.h"
#include "context_switch.h"
#include "interrupt.h"
#include "stdint.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "timer_raspi_reg.h"
#include "timer_sp804.h"
#include "util.h"

#ifndef TRACE_TIMER
#define TRACE_TIMER 0
#endif

static volatile unsigned int* base = (volatile unsigned int*) TIMER_RPI_BASE;

static void timer_raspi_clear_match();
static bool timer_raspi_is_match();
static void timer_raspi_isr();

#define RPI_INTNUM_TIMER 0

void timer_raspi_init() {
  interrupt_set_handler(RPI_INTNUM_TIMER, timer_raspi_isr);
  interrupt_enable(RPI_INTNUM_TIMER);
}

static void timer_raspi_isr() {
  sc_LOG_IF(TRACE_TIMER, "");
#if TRACE_TIMER
  timer_raspi_print_status();
#endif
  if (timer_raspi_is_match()) {
    sc_LOG_IF(TRACE_TIMER, "Clearing set match");
    timer_raspi_clear_match();
  } else {

    sc_LOG_IF(TRACE_TIMER, "Not our match, ignoring it.");
  }
  // Ignore other matching timers. I assume some other hardware is using them.
}

time timer_raspi_systemnow() {
  return timer_raspi_get_counter() * DURATION_US;
}

uint64_t timer_raspi_get_counter() {
  // Documentation says to always read CLO first.
  uint32_t lo = *(base + TIMER_RPI_CLO);
  uint32_t hi = *(base + TIMER_RPI_CHI);
  return (((uint64_t) hi) << 32) | ((uint64_t) lo);
}

void timer_raspi_set_deadline(time t) {
  sc_LOGF_IF(TRACE_TIMER, "t = %llu", t);
  uint32_t deadline_count = (uint32_t) (t / DURATION_US);
  *(base + TIMER_RPI_C3) = deadline_count;

#if TRACE_TIMER
  timer_raspi_print_status();
#endif

  if (t < timer_raspi_systemnow()) {
    // Deadline has already passed. Call the ISR directly in case we
    // don't get an interrupt.

    sc_LOG_IF(TRACE_TIMER, "Deadline already expired. Calling ISR immediately.");
    timer_raspi_isr();
  }
}

void timer_raspi_clear_deadline() {
  timer_raspi_set_deadline(0);
}

static void timer_raspi_clear_match() {
  *(base + TIMER_RPI_CS) = TIMER_RPI_CS_M3;
}

static bool timer_raspi_is_match() {
  return *(base + TIMER_RPI_CS) & TIMER_RPI_CS_M3;
}

void timer_raspi_set_timeout(duration_t d) {
  timer_raspi_set_deadline(timer_raspi_systemnow() + d);
}

void timer_raspi_print_status() {
    sc_LOG("");
    sc_printf("  CS       = %x\n",   *(base + TIMER_RPI_CS));
    // Documentation says to always read CLO first.
    sc_printf("  CLO      = %u\n",   *(base + TIMER_RPI_CLO));
    sc_printf("  CHI      = %u\n",   *(base + TIMER_RPI_CHI));
    sc_printf("  C0       = %u\n",   *(base + TIMER_RPI_C0));
    sc_printf("  C1       = %u\n",   *(base + TIMER_RPI_C1));
    sc_printf("  C2       = %u\n",   *(base + TIMER_RPI_C2));
    sc_printf("  C3       = %u\n",   *(base + TIMER_RPI_C3));
    sc_printf("  counter  = %llu\n", timer_raspi_get_counter());
    sc_printf("  systemnow = %llu\n", timer_raspi_systemnow());
    sc_printf("  systemnow ms = %llu\n", (timer_raspi_systemnow() / DURATION_MS));
}
