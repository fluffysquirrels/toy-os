#include "timer_sp804.h"

#include "arch_registers.h"
#include "arch_interrupt_numbers.h"
#include "interrupt.h"
#include "console.h"
#include "timer.h"
#include "timer_sp804_reg.h"

#ifndef TRACE_TIMER
#define TRACE_TIMER 0
#endif

struct timer_sp804_t {
  volatile uint32_t *addr;
  char *name;
};

#ifdef TIMER_SP804_BASE_0
static struct timer_sp804_t _timer0 = {
  .addr = (volatile uint32_t *) TIMER_SP804_BASE_0,
  .name = "TIMER0",
};
struct timer_sp804_t *timer_sp804_timer0 = &_timer0;
#endif // TIMER_SP804_BASE_0

#ifdef TIMER_SP804_BASE_1
static struct timer_sp804_t _timer1 = {
  .addr = (volatile uint32_t *) TIMER_SP804_BASE_1,
  .name = "TIMER1",
};
struct timer_sp804_t *timer_sp804_timer1 = &_timer1;
#endif // TIMER_SP804_BASE_1

#ifdef TIMER_SP804_BASE_2
static struct timer_sp804_t _timer2 = {
  .addr = (volatile uint32_t *) TIMER_SP804_BASE_2,
  .name = "TIMER2",
};
struct timer_sp804_t *timer_sp804_timer2 = &_timer2;
#endif // TIMER_SP804_BASE_2

#ifdef TIMER_SP804_BASE_3
static struct timer_sp804_t _timer3 = {
  .addr = (volatile uint32_t *) TIMER_SP804_BASE_3,
  .name = "TIMER3",
};
struct timer_sp804_t *timer_sp804_timer3 = &_timer3;
#endif // TIMER_SP804_BASE_3

#ifdef INTNUM_SP804_TIMER01
static void timer_sp804_isr_timer01();
#endif // INTNUM_SP804_TIMER01

void timer_sp804_init() {
#ifdef INTNUM_SP804_TIMER01
  interrupt_set_handler(INTNUM_SP804_TIMER01, timer_sp804_isr_timer01);
  interrupt_enable(INTNUM_SP804_TIMER01);
#endif // INTNUM_SP804_TIMER01
}

uint32_t timer_sp804_get_current(struct timer_sp804_t *timer) {
  return *(timer->addr + TIMER_VALUE);
}

void timer_sp804_set_timeout(struct timer_sp804_t *timer, uint32_t value) {
  LOGF_IF(TRACE_TIMER, "timer %s value=%u", timer->name, value);
#if TRACE_TIMER
  timer_sp804_log_timer_state(timer);
#endif
  *(timer->addr + TIMER_LOAD) = value;
  *(timer->addr + TIMER_CONTROL) =
    TIMER_CONTROL_EN |
    TIMER_CONTROL_ONESHOT |
    TIMER_CONTROL_32BIT |
    TIMER_CONTROL_INTEN;

#if TRACE_TIMER
  LOG("Timer set");
  timer_sp804_log_timer_state(timer);
#endif
}

void timer_sp804_set_periodic(struct timer_sp804_t *timer, uint32_t value) {
  LOGF_IF(TRACE_TIMER, "timer %s value=%u", timer->name, value);
#if TRACE_TIMER
  timer_sp804_log_timer_state(timer);
#endif
  *(timer->addr + TIMER_LOAD) = value;
  *(timer->addr + TIMER_BGLOAD) = value;
  *(timer->addr + TIMER_CONTROL) =
    TIMER_CONTROL_EN |
    TIMER_CONTROL_PERIODIC |
    TIMER_CONTROL_32BIT |
    TIMER_CONTROL_INTEN;
#if TRACE_TIMER
  timer_sp804_log_timer_state(timer);
#endif
}

#ifdef INTNUM_SP804_TIMER01
static void timer_sp804_isr_timer01() {
  LOG_IF(TRACE_TIMER, "start");
#if TRACE_TIMER
  timer_sp804_log_all_state();
#endif

  bool cleared_something = false;

  if(*(_timer0.addr + TIMER_MIS)) { /* Timer0 went off */
    cleared_something = true;
    *(_timer0.addr + TIMER_INTCLR) = 1; /* Clear interrupt */
    LOG_IF(TRACE_TIMER, "TIMER0 tick");
    timer_do_expired_callbacks();
  }

#if TIMER_SP804_BASE_1
  if(*(_timer1.addr + TIMER_MIS)) { /* Timer1 went off */
    cleared_something = true;
    *(_timer1.addr + TIMER_INTCLR) = 1; /* Clear interrupt */
    LOG_IF(TRACE_TIMER, "TIMER1 tick");
  }
#endif // TIMER_SP804_BASE_1

#if TRACE_TIMER
  timer_sp804_log_all_state();
#endif

  if(!cleared_something) {
    warn("*(TIMER0/1 + TIMER_MIS) was clear");
  }
}
#endif // INTNUM_SP804_TIMER01


void timer_sp804_log_all_state() {
#if TIMER_SP804_BASE_0
  timer_sp804_log_timer_state(&_timer0);
#endif
#if TIMER_SP804_BASE_1
  timer_sp804_log_timer_state(&_timer1);
#endif
#if TIMER_SP804_BASE_2
  timer_sp804_log_timer_state(&_timer2);
#endif
#if TIMER_SP804_BASE_3
  timer_sp804_log_timer_state(&_timer3);
#endif
}
void timer_sp804_log_timer_state(struct timer_sp804_t *timer) {
  LOGF("\n%s", timer->name);

  sc_print_uint32_mem("  TIMER_LOAD    ", timer->addr + TIMER_LOAD   );
  sc_print_uint32_mem("  TIMER_VALUE   ", timer->addr + TIMER_VALUE  );
  sc_print_uint32_mem("  TIMER_CONTROL ", timer->addr + TIMER_CONTROL);
  sc_print_uint32_mem("  TIMER_INTCLR  ", timer->addr + TIMER_INTCLR );
  sc_print_uint32_mem("  TIMER_RIS     ", timer->addr + TIMER_RIS    );
  sc_print_uint32_mem("  TIMER_MIS     ", timer->addr + TIMER_MIS    );
  sc_print_uint32_mem("  TIMER_BGLOAD  ", timer->addr + TIMER_BGLOAD );
}
