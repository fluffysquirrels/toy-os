#include "stdarg.h"
#include "console.h"
#include "timer.h"
#include "uart.h"
#include "util.h"

void sc_print_uint32_mem(char *name, volatile uint32_t *addr) {
  printf("%s @ %p = %lx\n", name, (void *) addr, *addr);
}

void sc_print_mem_region(volatile uint32_t *start_addr, int32_t bytes) {
  volatile uint32_t *addr = start_addr;
  int32_t remaining_bytes = bytes;
  while (remaining_bytes > 0) {
    sc_print_uint32_mem("", addr);

    addr++;
    remaining_bytes -= sizeof(*addr);
  }
}

int puts(const char *string) {
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

int sc_print_uint32_hex(uint32_t w) {
  puts("0x");
  sc_print_uint8_hex(w >> 28);
  sc_print_uint8_hex(w >> 24);
  sc_print_uint8_hex(w >> 20);
  sc_print_uint8_hex(w >> 16);
  sc_print_uint8_hex(w >> 12);
  sc_print_uint8_hex(w >> 8);
  sc_print_uint8_hex(w >> 4);
  sc_print_uint8_hex(w);
  return 10;
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

  return puts(buffCurr);
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

  return puts(buffCurr);
}

int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vprintf(format, args);
  va_end(args);
  return ret;
}

static bool peekeq(const char *buff, const char *test);

int vprintf(const char *format, va_list args) {
  int chars_written = 0;

  const char *curr = format;
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
      } else if (peekeq(curr, "lu")) {
        uint32_t n = va_arg(args, uint32_t);
        chars_written += sc_print_uint32_dec(n);
        curr += 2;
      } else if (peekeq(curr, "lx")) {
        uint32_t n = va_arg(args, uint32_t);
        chars_written += sc_print_uint32_hex(n);
        curr += 2;
      } else {
        switch (*curr) {
        case 'c':;
          char c = (char) va_arg(args, int);
          chars_written++;
          sc_putch(c);
          break;
        case 's':;
          char *s = va_arg(args, char *);
          chars_written += puts(s);
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
static bool peekeq(const char *buff, const char *test) {
  while(1) {
    if (*test == '\0') {
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
