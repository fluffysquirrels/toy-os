#include "versatilepb.h"
#include "asm.h"

void cputs(char *);
void cputch(char);
void first(void);

struct thread_t{

};

int main(void) {
  unsigned int first_stack[256];
  unsigned int *first_stack_start = first_stack + 256 - 16;
  /* TODO: Store these in a struct */
  first_stack_start[0] = 0x10;
  first_stack_start[1] = (unsigned int) &first;

  cputs("Hello, World from main!\n");
  first_stack_start = activate(first_stack_start);
  cputs("Back from user mode into main 1!\n");
  first_stack_start = activate(first_stack_start);
  cputs("Back from user mode into main 2!\n");

  halt();

  /*  while(1);*/
  return 0;
}

/* First user mode program */
void first(void) {
  cputs("In user mode 1\n");
  syscall();
  cputs("In user mode 2\n");
  syscall();
  while(1);
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
