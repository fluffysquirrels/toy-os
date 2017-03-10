#pragma once

#include "stdarg.h"
#include "stdbool.h"
#include "stdint.h"

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

void sc_logf(const char *file, uint32_t line, const char *func, char *format, ...);

#define ASSERT(cond) ASSERT_MSG(cond, "")

#define ASSERT_MSG(cond, msg)                                       \
  if (!(cond)) {                                                    \
    sc_LOGF("Assert failed: %s\n"                                   \
            "    %s\n"                                              \
            "Panic.\n",                                             \
            #cond, msg);                                            \
    halt();                                                         \
  }


// Assert lhs op rhs.
// `op' should be a binary condition operator, like `<=' or `!='.
// `lhs' and `rhs' should both be integer-valued expressions that
// can be cast to uint64_t.
#define ASSERT_INT_BINOP(lhs, op, rhs)                           \
  if (!((lhs) op (rhs))) {                                       \
    sc_LOGF("Assert failed: needed %s %s %s\n"                   \
            "  left side  %s = %llu\n"                           \
            "  right side %s = %llu\n",                          \
            #lhs, #op, #rhs,                                     \
            #lhs, (uint64_t) (lhs),                              \
            #rhs, (uint64_t) (rhs));                             \
    halt();                                                      \
  }

#define PANIC(str) PANICF("%s", str)

#define PANICF(format, ...)                         \
  sc_printf("panic in %s() @ %s:%s : "              \
            , __func__, __FILE__, STR(__LINE__));   \
  sc_printf(format, __VA_ARGS__);                   \
  sc_puts("\n");                                    \
  halt();

#define sc_LOG_IF(cond, msg)\
  sc_LOGF_IF(cond, "%s", msg)

#define sc_LOGF_IF(cond, format, ...)\
  if (cond) {\
    sc_LOGF(format, __VA_ARGS__);\
  }

#define sc_LOG(msg)\
  sc_LOGF("%s", msg)

#define sc_LOGF(format, ...)\
  sc_logf(__FILE__, __LINE__, __func__, format, __VA_ARGS__)

void halt() __attribute__ ((noreturn));
void warn(char *string);

void sc_print_uint32_mem(char *name, volatile uint32_t *addr);
void sc_print_mem_region(volatile uint32_t *start_addr, int32_t bytes);

int sc_puts(char *string);
void sc_putch(char ch);

void sc_print_uint32_hex(uint32_t w);
void sc_print_uint8_hex(char x);
int sc_print_uint32_dec(uint32_t d);
int sc_print_uint64_dec(uint64_t u);

int sc_printf(char *format, ...);
int sc_vprintf(char *format, va_list args);
