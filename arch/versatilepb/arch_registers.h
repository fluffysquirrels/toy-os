#pragma once

#define UART_BASE_0 0x101f1000
#define UART_BASE_1 0x101f2000
#define UART_BASE_2 0x101f3000
#define UART_BASE_3 0x10009000

// Hacky defines to keep raspi code compiling.
// TODO: The real solution is to have the Makefile not compile raspi code at all.
#define TIMER_RPI_BASE 0x0
