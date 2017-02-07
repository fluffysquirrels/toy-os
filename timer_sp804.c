#include "timer_sp804.h"

#include "arch_registers.h"
#include "interrupt.h"
#include "rtc_pl031.h"
#include "synchronous_console.h"
#include "timer.h"
#include "timer_sp804_reg.h"

#ifndef TRACE_SP804
#define TRACE_SP804 0
#endif

struct timer_sp804_t {
  volatile unsigned int *addr;
  char *name;
};

#ifdef TIMER_SP804_BASE_0
static struct timer_sp804_t _timer0 = {
  .addr = (volatile unsigned int *) TIMER_SP804_BASE_0,
  .name = "TIMER0",
};
struct timer_sp804_t *timer_sp804_timer0 = &_timer0;
#endif // TIMER_SP804_BASE_0

#ifdef TIMER_SP804_BASE_1
static struct timer_sp804_t _timer1 = {
  .addr = (volatile unsigned int *) TIMER_SP804_BASE_1,
  .name = "TIMER1",
};
struct timer_sp804_t *timer_sp804_timer1 = &_timer1;
#endif // TIMER_SP804_BASE_1

#ifdef TIMER_SP804_BASE_2
static struct timer_sp804_t _timer2 = {
  .addr = (volatile unsigned int *) TIMER_SP804_BASE_2,
  .name = "TIMER2",
};
struct timer_sp804_t *timer_sp804_timer2 = &_timer2;
#endif // TIMER_SP804_BASE_2

#ifdef TIMER_SP804_BASE_3
static struct timer_sp804_t _timer3 = {
  .addr = (volatile unsigned int *) TIMER_SP804_BASE_3,
  .name = "TIMER3",
};
struct timer_sp804_t *timer_sp804_timer3 = &_timer3;
#endif // TIMER_SP804_BASE_3

static void timer_sp804_isr_timer01();

void timer_sp804_init() {
  interrupt_set_handler(PIC_INTNUM_TIMER01, timer_sp804_isr_timer01);
  interrupt_enable(PIC_INTNUM_TIMER01);
}

uint32_t timer_sp804_get_current(struct timer_sp804_t *timer) {
  return *(timer->addr + TIMER_VALUE);
}

void timer_sp804_set_timeout(struct timer_sp804_t *timer, uint32_t value) {
  sc_LOGF_IF(TRACE_SP804, "timer %s value=%u", timer->name, value);
#if TRACE_SP804
  timer_sp804_log_timer_state(timer);
#endif
  *(timer->addr + TIMER_LOAD) = value;
  *(timer->addr + TIMER_CONTROL) =
    TIMER_CONTROL_EN |
    TIMER_CONTROL_ONESHOT |
    TIMER_CONTROL_32BIT |
    TIMER_CONTROL_INTEN;

#if TRACE_SP804
  sc_LOG("Timer set");
  timer_sp804_log_timer_state(timer);
#endif
}

void timer_sp804_set_periodic(struct timer_sp804_t *timer, uint32_t value) {
  sc_LOGF_IF(TRACE_SP804, "timer %s value=%u", timer->name, value);
#if TRACE_SP804
  timer_sp804_log_timer_state(timer);
#endif
  *(timer->addr + TIMER_LOAD) = value;
  *(timer->addr + TIMER_BGLOAD) = value;
  *(timer->addr + TIMER_CONTROL) =
    TIMER_CONTROL_EN |
    TIMER_CONTROL_PERIODIC |
    TIMER_CONTROL_32BIT |
    TIMER_CONTROL_INTEN;
#if TRACE_SP804
  timer_sp804_log_timer_state(timer);
#endif
}

void timer_sp804_isr_timer01() {
  sc_LOG_IF(TRACE_SP804, "start");
#if TRACE_SP804
  timer_sp804_log_all_state();
#endif

  if(*(_timer0.addr + TIMER_MIS)) { /* Timer0 went off */
    *(_timer0.addr + TIMER_INTCLR) = 1; /* Clear interrupt */
    sc_LOG_IF(TRACE_SP804, "TIMER0 tick");
    timer_do_expired_callbacks();
    return;
  }

#if TIMER_SP804_BASE_1
  if(*(_timer1.addr + TIMER_MIS)) { /* Timer1 went off */
    *(_timer1.addr + TIMER_INTCLR) = 1; /* Clear interrupt */
    sc_LOG_IF(TRACE_SP804, "TIMER1 tick");
    return;
  }
#endif // TIMER_SP804_BASE_1

  warn("*(TIMER0/1 + TIMER_MIS) was clear");
}

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
  sc_LOGF("%s", timer->name);

  sc_printf("  TIMER_LOAD    = %x\n", *(timer->addr + TIMER_LOAD   ));
  sc_printf("  TIMER_VALUE   = %x\n", *(timer->addr + TIMER_VALUE  ));
  sc_printf("  TIMER_CONTROL = %x\n", *(timer->addr + TIMER_CONTROL));
  sc_printf("  TIMER_INTCLR  = %x\n", *(timer->addr + TIMER_INTCLR ));
  sc_printf("  TIMER_RIS     = %x\n", *(timer->addr + TIMER_RIS    ));
  sc_printf("  TIMER_MIS     = %x\n", *(timer->addr + TIMER_MIS    ));
  sc_printf("  TIMER_BGLOAD  = %x\n", *(timer->addr + TIMER_BGLOAD ));
}
