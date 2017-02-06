#pragma once

#include "stdint.h"

#define TIMER_RPI_CS   (0x0  / sizeof(uint32_t))
#define TIMER_RPI_CLO  (0x4  / sizeof(uint32_t))
#define TIMER_RPI_CHI  (0x8  / sizeof(uint32_t))
#define TIMER_RPI_C0   (0xc  / sizeof(uint32_t))
#define TIMER_RPI_C1   (0x10 / sizeof(uint32_t))
#define TIMER_RPI_C2   (0x14 / sizeof(uint32_t))
#define TIMER_RPI_C3   (0x18 / sizeof(uint32_t))

#define TIMER_RPI_CS_M0 (1 << 0)
#define TIMER_RPI_CS_M1 (1 << 1)
#define TIMER_RPI_CS_M2 (1 << 2)
#define TIMER_RPI_CS_M3 (1 << 3)
