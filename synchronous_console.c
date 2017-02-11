#include "stdarg.h"
#include "synchronous_console.h"
#include "timer.h"
#include "uart.h"
#include "util.h"

void assert(bool cond, char *string) {
  assertf(cond, "%s", string);
}

void assertf(bool cond, char *format, ...) {
  if (!cond) {
    va_list args;
    va_start(args, format);
    sc_puts("assert failed: ");
    sc_vprintf(format, args);
    sc_puts("\npanic\n");
    va_end(args);
    halt();
  }
}

void panic(char *string) {
  panicf("%s", string);
}

void panicf(char *format, ...) {
  va_list args;
  va_start(args, format);
  sc_puts("panic: ");
  sc_vprintf(format, args);
  sc_puts("\n");
  halt();
}

void halt() {
  while (1) {}
}

void warn(char *string) {
  sc_puts("warn: ");
  sc_puts(string);
  sc_puts("\n");
}

void sc_logf(const char *file, uint32_t line, const char *func, char *format, ...) {
  sc_printf("%u %s:%u: %s(): ", (uint32_t) (timer_systemnow() / DURATION_MS), file, line, func);
  va_list args;
  va_start(args, format);
  sc_vprintf(format, args);
  va_end(args);
  sc_puts("\n");
}

void sc_print_uint32_memv(char *name, volatile uint32_t *addr) {
  sc_printf("%s @ %x = %x\n", name, addr, *addr);
}

void sc_print_mem_region(volatile uint32_t *start_addr, int32_t bytes) {
  volatile uint32_t *addr = start_addr;
  int32_t remaining_bytes = bytes;
  while (remaining_bytes > 0) {
    sc_print_uint32_memv("", addr);

    addr++;
    remaining_bytes -= sizeof(*addr);
  }
}

int sc_puts(char *string) {
  int bytes_written = 0;
  while(*string) {
    sc_putch(*string);
    string++;
    bytes_written++;
  }

  return bytes_written;
}

void sc_putch(char ch) {
  uart_sync_putch(uart_0, ch);
}

void sc_print_uint32_hex(uint32_t w) {
  sc_puts("0x");
  sc_print_uint8_hex(w >> 28);
  sc_print_uint8_hex(w >> 24);
  sc_print_uint8_hex(w >> 20);
  sc_print_uint8_hex(w >> 16);
  sc_print_uint8_hex(w >> 12);
  sc_print_uint8_hex(w >> 8);
  sc_print_uint8_hex(w >> 4);
  sc_print_uint8_hex(w);
}

void sc_print_uint8_hex(char x) {
  x &= 0xf;
  if(x < 10) {
    sc_putch('0' + x);
  } else if (x < 16) {
    sc_putch('a' + (x - 10));
  } // No other cases
}

int sc_print_uint32_dec(uint32_t u) {
  if (u == 0) {
    sc_putch('0');
    return 1;
  }

  // UINT32_MAX is ~4 billion, which needs 10 decimal digits to print,
  // +1 for a terminating null means we need an 11 byte buffer.
  char buff[11] = { 0 };

  char *buffCurr = buff + sizeof(buff) - 2;
  while(1) {
    char digit = '0' + (u % 10);
    *buffCurr = digit;
    u /= 10;
    if (u == 0) {
      break;
    }
    buffCurr--;
    ASSERT(buffCurr >= buff);
  }

  return sc_puts(buffCurr);
}

int sc_print_uint64_dec(uint64_t u) {
  if (u == 0) {
    sc_putch('0');
    return 1;
  }

  // UINT64_MAX is 10^19, which needs 20 decimal digits to print,
  // +1 for a terminating null means we need a 21 byte buffer.
  char buff[21] = { 0 };

  char *buffCurr = buff + sizeof(buff) - 2;
  while(1) {
    char digit = '0' + (u % 10);
    *buffCurr = digit;
    u /= 10;
    if (u == 0) {
      break;
    }
    buffCurr--;
    ASSERT(buffCurr >= buff);
  }

  return sc_puts(buffCurr);
}

int sc_printf(char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = sc_vprintf(format, args);
  va_end(args);
  return ret;
}

static bool peekeq(char *buff, char *test);

int sc_vprintf(char *format, va_list args) {
  int chars_written = 0;

  char *curr = format;
  while (*curr != '\0') {
    if (*curr != '%') {
      sc_putch(*curr);
      curr++;
      chars_written++;
      continue;
    } else {
      // *curr == '%'
      curr++;
      if (*curr == '\0') {
        // Invalid '%' at the end of string.
        return -1;
      }

      if (peekeq(curr, "llu")) {
        uint64_t n = va_arg(args, uint64_t);
        chars_written += sc_print_uint64_dec(n);
        curr += 3;
      } else {
        switch (*curr) {
        case 'c':;
          char c = (char) va_arg(args, int);
          chars_written++;
          sc_putch(c);
          break;
        case 's':;
          char *s = va_arg(args, char *);
          chars_written += sc_puts(s);
          break;
        case 'u':;
          int u = va_arg(args, unsigned int);
          chars_written += sc_print_uint32_dec(u);
          break;
        case 'x':;
          unsigned int d = va_arg(args, unsigned int);
          sc_print_uint32_hex(d);
          chars_written += 10;
          break;
        default:;
          PANICF("Invalid parameter specifier in vprintf: %c", *curr);
          // Invalid parameter specifier.
          return -1;
        }

        curr++;
      }
    }
  } // while (*curr != '\0')


  return chars_written;
}

// param buff is a null terminated string
// param test is a null terminated string
// The beginning of buff should match all of test.
static bool peekeq(char *buff, char *test) {
  while(1) {
    if (*test == NULL) {
      return true;
    }
    // *test != NULL
    if (*buff != *test) {
      return false;
    }
    // *buff == *test

    buff++;
    test++;
  }
}
