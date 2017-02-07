// This is a driver for the ARM SP804 high resolution timer.
//
// See ARM Dual-Timer Module (SP804) Technical Reference Manual:
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html

#pragma once

#include "syscalls.h"

// QEMU runs the timers at 1MHz.
#define TIMER_SP804_TICKS_PER_MS (1000)

// Rename SCHEDULER_TIMER
#define TIMER_SP804_TIMEOUT_TIMER    (timer_sp804_timer0)
#define TIMER_SP804_1S_COUNTDOWN_TIMER (timer_sp804_timer1)

struct timer_sp804_t;
extern struct timer_sp804_t *timer_sp804_timer0;
extern struct timer_sp804_t *timer_sp804_timer1;
extern struct timer_sp804_t *timer_sp804_timer2;
extern struct timer_sp804_t *timer_sp804_timer3;

uint32_t timer_sp804_get_current(struct timer_sp804_t *timer);
void timer_sp804_set_timeout(struct timer_sp804_t *timer, uint32_t value);
void timer_sp804_set_periodic(struct timer_sp804_t *timer, uint32_t value);
void timer_sp804_init();

void timer_sp804_log_all_state();
void timer_sp804_log_timer_state(struct timer_sp804_t *timer);
