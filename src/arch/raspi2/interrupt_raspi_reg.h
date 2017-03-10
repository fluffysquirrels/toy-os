#pragma once

#define IC_RASPI_PENDING_BASIC  (0x200 / sizeof(uint32_t))
#define IC_RASPI_PENDING_1      (0x204 / sizeof(uint32_t))
#define IC_RASPI_PENDING_2      (0x208 / sizeof(uint32_t))

#define IC_RASPI_FIQ_CONTROL    (0x20C / sizeof(uint32_t))

#define IC_RASPI_ENABLE_1       (0x210 / sizeof(uint32_t))
#define IC_RASPI_ENABLE_2       (0x214 / sizeof(uint32_t))
#define IC_RASPI_ENABLE_BASIC   (0x218 / sizeof(uint32_t))

#define IC_RASPI_DISABLE_1      (0x21C / sizeof(uint32_t))
#define IC_RASPI_DISABLE_2      (0x220 / sizeof(uint32_t))
#define IC_RASPI_DISABLE_BASIC  (0x224 / sizeof(uint32_t))
