#pragma once

#include "stdint.h"

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

void logf_(const char *file, uint32_t line, const char *func, char *format, ...);

#define ASSERT(cond) ASSERT_MSG(cond, "")

#define ASSERT_MSG(cond, msg)                                       \
  if (!(cond)) {                                                    \
    LOGF("Assert failed: %s\n"                                      \
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
    LOGF("Assert failed: needed %s %s %s\n"                      \
            "  left side  %s = %llu\n"                           \
            "  right side %s = %llu\n",                          \
            #lhs, #op, #rhs,                                     \
            #lhs, (uint64_t) (lhs),                              \
            #rhs, (uint64_t) (rhs));                             \
    halt();                                                      \
  }

#define PANIC(str) PANICF("%s", str)

#define PANICF(format, ...)                         \
  printf("panic in %s() @ %s:%s : "                 \
         , __func__, __FILE__, STR(__LINE__));      \
  printf(format "\n", __VA_ARGS__);                 \
  halt();

#define LOG_IF(cond, msg)                       \
  LOGF_IF(cond, "%s", msg)

#define LOGF_IF(cond, format, ...)              \
  if (cond) {                                   \
    LOGF(format, __VA_ARGS__);                  \
  }

#define LOG(msg)                                \
  LOGF("%s", msg)

#define LOGF(format, ...)                                   \
  logf_(__FILE__, __LINE__, __func__, format, __VA_ARGS__)

void halt() __attribute__ ((noreturn));
void warn(char *string);
