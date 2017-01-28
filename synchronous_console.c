#include "synchronous_console.h"
#include "versatilepb.h"

void panic(char *string) {
  cputs("panic: ");
  cputs(string);
  cputs("\n");
  while(1) {}
}

void warn(char *string) {
  cputs("warn: ");
  cputs(string);
  cputs("\n");
}

void cputs(char *string) {
  while(*string) {
    cputch(*string);
    string++;
  }
}

void cputch(char ch) {
  while(*(UART0 + UARTFR) & UARTFR_TXFF);
  *UART0 = ch;
}

void cprint_word(unsigned int w) {
  cputs("0x");
  cprint_hex(w >> 28);
  cprint_hex(w >> 24);
  cprint_hex(w >> 20);
  cprint_hex(w >> 16);
  cprint_hex(w >> 12);
  cprint_hex(w >> 8);
  cprint_hex(w >> 4);
  cprint_hex(w);
}

void cprint_hex(char x) {
  x &= 0xf;
  if(x < 10) {
    cputch('0' + x);
  } else if (x < 16) {
    cputch('a' + (x - 10));
  } else {
    /* Should assert. */
    cputch('?');
  }
}
