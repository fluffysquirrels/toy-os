#pragma once
#include "stdarg.h"
#include "stdbool.h"

void assert(bool cond, char *string);
void assertf(bool cond, char *format, ...);
void panic(char *string);
void panicf(char *format, ...);
void warn(char *string);
int sc_puts(char *string);
void sc_putch(char ch);
void sc_print_uint32_hex(unsigned int w);
void sc_print_uint8_hex(char x);
int sc_printf(char *format, ...);
int sc_vprintf(char *format, va_list args);
