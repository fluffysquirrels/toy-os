#include "versatilepb.h"
#include "asm.h"

struct thread_t{
  unsigned int cpsr;
  void (*pc)(void);
  void* sp;
};

void cputs(char *);
void cputch(char);
void first(void);
void first_sub(unsigned int);
void cprint_thread(struct thread_t*);
void cprint_hex(char);
void cprint_word(unsigned int);

int main(void) {
  unsigned int first_stack[256];
  unsigned int *first_stack_start = first_stack + 256 - 16;

  struct thread_t first_thread = {
    .cpsr = 0x10,
    .pc = &first,
    .sp = first_stack_start,
  };

  cputs("Hello, World from main!\n");
  cprint_thread(&first_thread);
  activate(&first_thread);
  cputs("Back from user mode into main 1!\n");
  cprint_thread(&first_thread);
  activate(&first_thread);
  cputs("Back from user mode into main 2!\n");
  cprint_thread(&first_thread);
  activate(&first_thread);
  cputs("Back from user mode into main 3!\n");
  cprint_thread(&first_thread);

  halt();

  /*  while(1);*/
  return 0;
}

/* First user mode program */
void first(void) {
  cputs("In user mode 1\n");
  syscall();
  first_sub(17);
  cputs("In user mode 2\n");
  syscall();
  cputs("In user mode 3\n");
  while(1);
}

void first_sub(unsigned int arg1) {
  cputs("In first_sub 1 arg1=");
  cprint_word(arg1);
  cputs("\n");
  syscall();
  cputs("In first_sub 2 arg1=");
  cprint_word(arg1);
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

void cprint_thread(struct thread_t *thread) {
  cputs("thread {\n");
  cputs("  .cpsr = ");
  cprint_word(thread->cpsr);
  cputs("\n");
  cputs("  .pc = ");
  cprint_word((unsigned int) thread->pc);
  cputs("\n");
  cputs("  .sp = ");
  cprint_word((unsigned int) thread->sp);
  cputs("\n");
  cputs("}\n");
}
