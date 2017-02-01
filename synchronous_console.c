#include "stdarg.h"
#include "synchronous_console.h"
#include "uart.h"
#include "util.h"

void assert(bool cond, char *string) {
  if(!cond) {
    panic(string);
  }
}

void panic(char *string) {
  sc_puts("panic: ");
  sc_puts(string);
  sc_puts("\n");
  while(1) {}
}

void warn(char *string) {
  sc_puts("warn: ");
  sc_puts(string);
  sc_puts("\n");
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

void sc_print_uint32_hex(unsigned int w) {
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
  } else {
    /* Should assert. */
    sc_putch('?');
  }
}

int sc_printf(char *format, ...) {
  int chars_written = 0;

  va_list args;
  va_start(args, format);

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
      case 'x':;
        unsigned int d = va_arg(args, unsigned int);
        sc_print_uint32_hex(d);
        chars_written += 10;
        break;
      default:
        // Invalid parameter specifier.
        return -1;
      }

      curr++;
    }
  }

  va_end(args);

  return chars_written;
}
