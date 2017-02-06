#include "timer_raspi.h"

#include "stdint.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "util.h"

#define TIMER_RPI_BASE 0x3F003000
// #define TIMER_RPI_BASE 0x20003000
// #define TIMER_RPI_BASE 0x7E003000

static volatile unsigned int* base = (volatile unsigned int*) TIMER_RPI_BASE;

#define TIMER_RPI_CS   (0x0  / sizeof(unsigned int))
#define TIMER_RPI_CLO  (0x4  / sizeof(unsigned int))
#define TIMER_RPI_CHI  (0x8  / sizeof(unsigned int))
#define TIMER_RPI_C0   (0xc  / sizeof(unsigned int))
#define TIMER_RPI_C1   (0x10 / sizeof(unsigned int))
#define TIMER_RPI_C2   (0x14 / sizeof(unsigned int))
#define TIMER_RPI_C3   (0x18 / sizeof(unsigned int))

#define TIMER_RPI_CS_M0 (1 << 0)
#define TIMER_RPI_CS_M1 (1 << 1)
#define TIMER_RPI_CS_M2 (1 << 2)
#define TIMER_RPI_CS_M3 (1 << 3)

void timer_raspi_init() {

}

void timer_raspi_spam() {
  while (1) {
    for (uint64_t i = 0; i < 20000000LL; i++) { }
    timer_raspi_print_status();
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

void timer_raspi_set_timeout(duration_t d) {
  UNUSED(d);
  PANIC("WIP");
}
