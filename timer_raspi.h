#pragma once

#include "syscalls.h"

void timer_raspi_spam();
void timer_raspi_print_status();
void timer_raspi_init();
time timer_raspi_systemnow();
uint64_t timer_raspi_get_counter();
