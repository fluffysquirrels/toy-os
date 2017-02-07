#pragma once

#define UART_BASE_0 0x101f1000
#define UART_BASE_1 0x101f2000
#define UART_BASE_2 0x101f3000
#define UART_BASE_3 0x10009000

#define TIMER_SP804_BASE_0 0x101E2000
#define TIMER_SP804_BASE_1 0x101E2020
#define TIMER_SP804_BASE_2 0x101E3000
#define TIMER_SP804_BASE_3 0x101E3020

// Hacky defines to keep other arch's code compiling.
// TODO: The real solution is to have the Makefile not compile other arch's code at all.
#define TIMER_RPI_BASE 0x0
#define IC_RPI_BASE    0x0
