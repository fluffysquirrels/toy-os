#pragma once

#include "log.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stdint.h"

void sc_print_uint32_mem(char *name, volatile uint32_t *addr);
void sc_print_mem_region(volatile uint32_t *start_addr, int32_t bytes);

int puts(char *string);
void sc_putch(char ch);

int sc_print_uint32_hex(uint32_t w);
void sc_print_uint8_hex(char x);
int sc_print_uint32_dec(uint32_t d);
int sc_print_uint64_dec(uint64_t u);

int printf(char *format, ...);
int vprintf(char *format, va_list args);
