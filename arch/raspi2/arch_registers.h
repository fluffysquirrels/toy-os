#pragma once

#define UART_BASE_0        0x3F201000
#define TIMER_RPI_BASE     0x3F003000
#define IC_RPI_BASE        0x3F00B000
#define TIMER_SP804_BASE_0 0x3F00B400

// Hacky defines to keep other arch's code compiling.
// TODO: The real solution is to have the Makefile not compile other arch's code at all.
#define TIMER_SP804_BASE_1 0x0
