#include "synchronous_console.h"
#include "versatilepb.h"

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

void sc_puts(char *string) {
  while(*string) {
    sc_putch(*string);
    string++;
  }
}

void sc_putch(char ch) {
  while(*(UART0 + UARTFR) & UARTFR_TXFF);
  *UART0 = ch;
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
