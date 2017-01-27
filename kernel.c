#include "asm.h"
#include "asm_constants.h"
#include "versatilepb.h"

struct thread_t{
  unsigned int cpsr;
  void (*pc)(void);
  void* sp;
};

void enable_timer0_interrupt(void);
void start_periodic_timer0(void);

void first(void);
void first_sub(unsigned int);
void second(void);

void cputs(char *);
void cputch(char);
void cprint_thread(struct thread_t*);
void cprint_hex(char);
void cprint_word(unsigned int);

int main(void) {
  unsigned int first_stack[256];
  unsigned int *first_stack_start = first_stack + 256 - 16;
  unsigned int second_stack[256];
  unsigned int *second_stack_start = second_stack + 256 - 16;

  struct thread_t first_thread = {
    .cpsr = 0x10,
    .pc = &first,
    .sp = first_stack_start,
  };
  struct thread_t second_thread = {
    .cpsr = 0x10,
    .pc = &second,
    .sp = second_stack_start,
  };

  struct thread_t *threads[2] = {
    &first_thread,
    &second_thread,
  };

  cputs("Hello, World from main!\n");

  enable_timer0_interrupt();
  start_periodic_timer0();

  unsigned int thread_idx = 0;
  unsigned int num_threads = 2;

  while(1) {
    struct thread_t *thread = threads[thread_idx];
    /* Can't use % to calculate new thread_idx because that requires a
       runtime library function */
    thread_idx++;
    if(thread_idx >= num_threads) {
      thread_idx = 0;
    }
    //    cprint_thread(thread);

    unsigned int stop_reason = activate(thread);
    //    cputs("activate returned ");
    //    cprint_word(stop_reason);
    //    cputs("\n");
    if(stop_reason == ACTIVATE_RET_IRQ) {
      /* Handle interrupt */
      if(*(TIMER0 + TIMER_MIS)) { /* Timer0 went off */
        *(TIMER0 + TIMER_INTCLR) = 1; /* Clear interrupt */
        cputs("TIMER0 tick\n");
      }
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      /* Handle syscall */
    }
  }

  sleep();
  return 0;
}

void enable_timer0_interrupt() {
  *(PIC + VIC_INTENABLE) = PIC_TIMER01;
}

void start_periodic_timer0() {
  *(TIMER0 + TIMER_LOAD) = 100000;
  *(TIMER0 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
  *(TIMER0 + TIMER_BGLOAD) = 100000;
}

/* First user mode program */
void first(void) {
  //  cputs("In first() 1\n");
  syscall();
  unsigned int n = 17;
  while(1) {
    //    cputs("In first() 2\n");
    n++;
    first_sub(n);
    //    cputs("In first() 3\n");
    syscall();
  }
}

void first_sub(unsigned int arg1) {
  //  cputs("In first_sub 1 arg1=");
  //  cprint_word(arg1);
  //  cputs("\n");
  syscall();
  //  cputs("In first_sub 2 arg1=");
  //  cprint_word(arg1);
  //  cputs("\n");
}

/* Second user mode program */
void second(void) {
  //  cputs("In second() 1\n");
  //  syscall();
  while(1) {
    //cputs("In second() 2\n");
    //    syscall();
  }
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
