#pragma once
#include "stdbool.h"

void assert(bool cond, char *string);
void panic(char *string);
void warn(char *string);
void sc_puts(char *string);
void sc_putch(char ch);
void sc_print_uint32_hex(unsigned int w);
void sc_print_uint8_hex(char x);
