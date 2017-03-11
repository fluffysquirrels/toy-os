#include "log.h"

#include "stdarg.h"
#include "stdio.h"

void halt() {
  while (1) {}
}

void warn(char *string) {
  puts("warn: ");
  puts(string);
  puts("\n");
}

void logf_(const char *file, uint32_t line, const char *func, char *format, ...) {
  // TODO: Add time to log message.
  printf("%s:%lu: %s(): ",
         file, line, func);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  puts("\n");
}
