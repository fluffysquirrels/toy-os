#pragma once

#include "syscalls.h"

// arch_timer_common implementation
void timer_raspi_init();
time timer_raspi_systemnow();
void timer_raspi_set_timeout(duration_t d);
void timer_raspi_set_deadline(time t);

void timer_raspi_print_status();
uint64_t timer_raspi_get_counter();
void timer_raspi_clear_deadline();

struct arch_timer *timer_raspi_get_arch_timer();
