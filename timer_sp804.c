#include "interrupt.h"
#include "rtc_pl031.h"
#include "synchronous_console.h"
#include "timer.h"
#include "timer_sp804.h"
#include "timer_sp804_reg.h"

#ifndef TRACE_SP804
#define TRACE_SP804 0
#endif

struct timer_sp804_t {
  volatile unsigned int *addr;
  char *name;
};

static struct timer_sp804_t _timer0 = {
  .addr = (volatile unsigned int *) TIMER_BASE_0,
  .name = "TIMER0",
};
struct timer_sp804_t *timer_sp804_timer0 = &_timer0;

static struct timer_sp804_t _timer1 = {
  .addr = (volatile unsigned int *) TIMER_BASE_1,
  .name = "TIMER1",
};
struct timer_sp804_t *timer_sp804_timer1 = &_timer1;

static struct timer_sp804_t _timer2 = {
  .addr = (volatile unsigned int *) TIMER_BASE_2,
  .name = "TIMER2",
};
struct timer_sp804_t *timer_sp804_timer2 = &_timer2;


static struct timer_sp804_t _timer3 = {
  .addr = (volatile unsigned int *) TIMER_BASE_3,
  .name = "TIMER3",
};
struct timer_sp804_t *timer_sp804_timer3 = &_timer3;

static void isr_timer01();

void timer_sp804_init() {
  interrupt_set_handler(PIC_INTNUM_TIMER01, isr_timer01);
  interrupt_enable(PIC_INTNUM_TIMER01);
}

uint32_t timer_sp804_get_current(struct timer_sp804_t *timer) {
  return *(timer->addr + TIMER_VALUE);
}

void timer_sp804_set_timeout(struct timer_sp804_t *timer, uint32_t value) {
  sc_LOGF_IF(TRACE_SP804, "timer %s value=%u", timer->name, value);

  *(timer->addr + TIMER_LOAD) = value;
  *(timer->addr + TIMER_CONTROL) =
    TIMER_CONTROL_EN |
    TIMER_CONTROL_ONESHOT |
    TIMER_CONTROL_32BIT |
    TIMER_CONTROL_INTEN;
}

void timer_sp804_set_periodic(struct timer_sp804_t *timer, uint32_t value) {
  sc_LOGF_IF(TRACE_SP804, "timer %s value=%u", timer->name, value);

  *(timer->addr + TIMER_LOAD) = value;
  *(timer->addr + TIMER_BGLOAD) = value;
  *(timer->addr + TIMER_CONTROL) =
    TIMER_CONTROL_EN |
    TIMER_CONTROL_PERIODIC |
    TIMER_CONTROL_32BIT |
    TIMER_CONTROL_INTEN;
}

static void isr_timer01() {
  sc_LOG_IF(TRACE_SP804, "start");
  sc_LOGF_IF(TRACE_SP804, "TIMER0 current = %u", timer_sp804_get_current(timer_sp804_timer0));
  sc_LOGF_IF(TRACE_SP804, "TIMER1 current = %u", timer_sp804_get_current(timer_sp804_timer1));

  if(*(TIMER_BASE_0 + TIMER_MIS)) { /* Timer0 went off */
    *(TIMER_BASE_0 + TIMER_INTCLR) = 1; /* Clear interrupt */
    sc_LOG_IF(TRACE_SP804, "TIMER0 tick");
    timer_do_expired_callbacks();
  } else if(*(TIMER_BASE_1 + TIMER_MIS)) { /* Timer1 went off */
    *(TIMER_BASE_1 + TIMER_INTCLR) = 1; /* Clear interrupt */
    sc_LOG_IF(TRACE_SP804, "TIMER1 tick");
  } else {
    PANIC("*(TIMER0/1 + TIMER_MIS) was clear");
  }
}
