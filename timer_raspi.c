#include "timer_raspi.h"

#include "arch_registers.h"
#include "stdint.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "timer_raspi_reg.h"
#include "util.h"

static volatile unsigned int* base = (volatile unsigned int*) TIMER_RPI_BASE;

static void timer_raspi_clear_match();
static bool timer_raspi_is_match();
static void timer_raspi_isr();

void timer_raspi_init() {
  // TODO:  set_isr();
  //  set_interrupt_handler(dunno, timer_raspi_isr);
  //  interrupt_enable();
}

static void timer_raspi_isr() {
  PANIC("WIP");
}

void timer_raspi_spam() {
  timer_raspi_set_timeout(1 * DURATION_S);
  while (1) {
    for (uint64_t i = 0; i < 20000000LL; i++) { }
    timer_raspi_print_status();

    if (timer_raspi_is_match()) {
      sc_LOG("M3 pending");
      timer_raspi_print_status();
      sc_LOG("About to reset");
      // Reset M3
      timer_raspi_clear_match();
      timer_raspi_set_timeout(1 * DURATION_S);
      sc_LOG("M3 reset, next deadline set for 1s");
      timer_raspi_print_status();
    }
  }
}

time timer_raspi_systemnow() {
  return timer_raspi_get_counter() * DURATION_US;
}

uint64_t timer_raspi_get_counter() {
  uint32_t hi = *(base + TIMER_RPI_CHI);
  uint32_t lo = *(base + TIMER_RPI_CLO);
  return (((uint64_t) hi) << 32) | ((uint64_t) lo);
}

void timer_raspi_set_deadline(time t) {
  uint32_t deadline_count = (uint32_t) (t / DURATION_US);
  *(base + TIMER_RPI_C3) = deadline_count;

  if (t < timer_raspi_systemnow()) {
    // Deadline has already passed. Call the ISR directly in case we
    // don't get an interrupt.
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
