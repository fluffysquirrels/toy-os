#include "asm_constants.h"
#include "context_switch.h"
#include "syscalls.h"
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

void warn(char *);
void panic(char *);

void cputs(char *);
void cputch(char);
void cprint_thread(struct thread_t*);
void cprint_hex(char);
void cprint_word(unsigned int);

#define UNUSED(x) (void)(x)

#define STACK_SIZE 256
#define THREAD_LIMIT 2

#define TRACE_SCHEDULER 1

unsigned int stacks[THREAD_LIMIT][STACK_SIZE];
struct thread_t threads[THREAD_LIMIT];

int main(void) {

  (threads[0]) = (struct thread_t) {
    .cpsr = 0x10,
    .pc = &first,
    .sp = stacks[0] + STACK_SIZE - 16,
  };
  (threads[1]) = (struct thread_t){
    .cpsr = 0x10,
    .pc = &second,
    .sp = stacks[1] + STACK_SIZE - 16,
  };

  cputs("Hello, World from main!\n");

  enable_timer0_interrupt();
  start_periodic_timer0();

  unsigned int thread_idx = 0;
  unsigned int num_threads = 2;

  while(1) {
    struct thread_t *thread = &threads[thread_idx];
    /* Can't use % to calculate new thread_idx because that requires a
       runtime library function */
    thread_idx++;
    if(thread_idx >= num_threads) {
      thread_idx = 0;
    }

#if TRACE_SCHEDULER
    cputs("main() thread_idx=");
    cprint_word(thread_idx);
    cputs("\n");
    cprint_thread(thread);
#endif // TRACE_SCHEDULER

    unsigned int stop_reason = activate(thread);

#if TRACE_SCHEDULER
    cputs("activate returned ");
    cprint_word(stop_reason);
    cputs("\n");
#endif // TRACE_SCHEDULER

    if(stop_reason == ACTIVATE_RET_IRQ) {
      /* Handle interrupt */
      if(*(TIMER0 + TIMER_MIS)) { /* Timer0 went off */
        *(TIMER0 + TIMER_INTCLR) = 1; /* Clear interrupt */
#if TRACE_SCHEDULER
        cputs("TIMER0 tick\n");
#endif // TRACE_SCHEDULER
      }
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      /* Handle syscall */
      unsigned int syscall_num = 1; // TODO: Extract this from thread.
      switch(syscall_num) {
      case SYSCALL_NUM_YIELD:
#if TRACE_SCHEDULER
        cputs("In yield\n");
#endif // TRACE_SCHEDULER
        break;
      default:
        warn("syscall with unknown syscall_num = ");
        cputs("  ");
        cprint_word(syscall_num);
        cputs("\n");
      }
    }
  }

  /* Not reached */
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
  cputs("Start first()\n");
  sys_yield();
  unsigned int n = 17;
  while(1) {
    cputs("In first() loop\n");
    n++;
    first_sub(n);
    sys_yield();
  }
}

void first_sub(unsigned int arg1) {
  UNUSED(arg1);
  sys_yield();
}

/* Second user mode program */
void second(void) {
  cputs("Start second()\n");
  while(1) {
  }
}

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
