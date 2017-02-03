// This is a driver for the ARM PL031 Real Time Clock

// See ARM PrimeCell™ Technical Reference Manual Real Time Clock (PL031):
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html

#pragma once

#include "syscall.h"

uint32_t rtc_pl031_get_current();
void rtc_pl031_init();
