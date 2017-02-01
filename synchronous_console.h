#pragma once
#include "stdarg.h"
#include "stdbool.h"


#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

#define ASSERT(cond)\
  if (!(cond)) {\
    sc_printf("assert failed in %s() @ %s:%s : %s\n"\
              "panic\n", __func__, __FILE__, STR(__LINE__), #cond);\
    halt();\
  }

#define PANIC(str) PANICF("%s", str)

#define PANICF(format, ...)\
  sc_printf("panic in %s() @ %s:%s : "\
            , __func__, __FILE__, STR(__LINE__));\
  sc_printf(format, __VA_ARGS__);\
  sc_puts("\n");\
  halt();

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

void assert(bool cond, char *string);
void assertf(bool cond, char *format, ...);
void panic(char *string);
void panicf(char *format, ...);
void halt();
void warn(char *string);
int sc_puts(char *string);
void sc_putch(char ch);
void sc_print_uint32_hex(unsigned int w);
void sc_print_uint8_hex(char x);
int sc_printf(char *format, ...);
int sc_vprintf(char *format, va_list args);
