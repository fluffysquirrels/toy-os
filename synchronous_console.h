#pragma once

#include "stdarg.h"
#include "stdbool.h"
#include "stdint.h"


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

#define sc_LOG_IF(cond, msg)\
  sc_LOGF_IF(cond, "%s", msg)


#define sc_LOGF_IF(cond, format, ...)\
  if (cond) {\
    sc_LOGF(format, __VA_ARGS__);\
  }

#define sc_LOG(msg)\
  sc_LOGF("%s", msg)

/* // #define sc_LOGF(format, ...)\ */
/* //   sc_printf("%s:%s: %s(): ", __FILE__, STR(__LINE__), __func__);    \ */
/* //   sc_printf(format, __VA_ARGS__);                                   \ */
/* //   sc_puts("\n"); */

#define sc_LOGF(format, ...)\
  sc_logf(__FILE__, __LINE__, __func__, format, __VA_ARGS__)

void sc_logf(const char *file, uint32_t line, const char *func, char *format, ...);

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

void assert(bool cond, char *string);
void assertf(bool cond, char *format, ...);
void panic(char *string);
void panicf(char *format, ...);
void halt();
void warn(char *string);

void sc_print_uint32_memv(char *name, volatile uint32_t *addr);

int sc_puts(char *string);
void sc_putch(char ch);

void sc_print_uint32_hex(uint32_t w);
void sc_print_uint8_hex(char x);
int sc_print_uint32_dec(uint32_t d);
int sc_print_uint64_dec(uint64_t u);

int sc_printf(char *format, ...);
int sc_vprintf(char *format, va_list args);
