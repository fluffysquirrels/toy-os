#include "asm_constants.h"
#include "context_switch.h"
#include "syscalls.h"
#include "versatilepb.h"

struct thread_t{
  /* Referenced by offset from assembly in context_switch.S, be careful! */
  unsigned int cpsr;
  unsigned int registers[16];
  /* End of Referenced by offset from assembly */

  unsigned int state;
};

int main(void);
void handle_syscall(struct thread_t*);
void handle_interrupt(struct thread_t*);

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

void *memset(void*, int, int);

#define UNUSED(x) (void)(x)

#define STACK_SIZE 256
#define THREAD_LIMIT 2

#define TRACE_SCHEDULER 1

unsigned int stacks[THREAD_LIMIT][STACK_SIZE];
struct thread_t threads[THREAD_LIMIT];

int main(void) {

  (threads[0]) = (struct thread_t) {
    .cpsr = 0x10,
    .registers = { 0 },
    .state = 0
  };
  threads[0].registers[13] = (unsigned int) (stacks[0] + STACK_SIZE - 16);
  threads[0].registers[15] = (unsigned int) &first;
  (threads[1]) = (struct thread_t){
    .cpsr = 0x10,
    .registers = { 0 },
    .state = 0
  };
  threads[1].registers[13] = (unsigned int) stacks[1] + STACK_SIZE - 16;
  threads[1].registers[15] = (unsigned int) &second;

  cputs("Hello, World from main!\n");

  enable_timer0_interrupt();
  start_periodic_timer0();

  unsigned int thread_idx = 0;
  unsigned int num_threads = 2;

  while(1) {
    struct thread_t *thread = &threads[thread_idx];
    /* Can't use % to calculate new thread_idx because that requires a
       runtime library function */

#if TRACE_SCHEDULER
    cputs("main() thread_idx=");
    cprint_word(thread_idx);
    cputs("\n");
    cprint_thread(thread);
#endif // TRACE_SCHEDULER

    thread_idx++;
    if(thread_idx >= num_threads) {
      thread_idx = 0;
    }

    unsigned int stop_reason = activate(thread);

#if TRACE_SCHEDULER
    cputs("activate returned ");
    cprint_word(stop_reason);
    if(stop_reason == ACTIVATE_RET_IRQ) {
      cputs(" = ACTIVATE_RET_IRQ");
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      cputs(" = ACTIVATE_RET_SYSCALL");
    }
    cputs("\n");
#endif // TRACE_SCHEDULER

    if(stop_reason == ACTIVATE_RET_IRQ) {
      handle_interrupt(thread);
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      handle_syscall(thread);
    }
  }

  /* Not reached */
  return 0;
}

void handle_interrupt(struct thread_t* thread) {
  UNUSED(thread);

  if(*(TIMER0 + TIMER_MIS)) { /* Timer0 went off */
    *(TIMER0 + TIMER_INTCLR) = 1; /* Clear interrupt */
#if TRACE_SCHEDULER
    cputs("TIMER0 tick\n");
#endif // TRACE_SCHEDULER
  } else {
    warn("Unknown interrupt went unacknowledged");
  }
}

void handle_syscall(struct thread_t* thread) {
  unsigned int syscall_num = thread->registers[12];
#if TRACE_SCHEDULER
  cputs("In syscall handler syscall_num = ");
  cprint_word(syscall_num);
  cputs("\n");
#endif // TRACE_SCHEDULER
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

void enable_timer0_interrupt() {
  *(PIC + VIC_INTENABLE) = PIC_TIMER01;
}

void start_periodic_timer0() {
  *(TIMER0 + TIMER_LOAD) = 1000000;
  *(TIMER0 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
  *(TIMER0 + TIMER_BGLOAD) = 1000000;
}

/* First user mode program */
void first(void) {
  cputs("Start first()\n");
  __asm__ volatile(
    "mov    r0,  #10 " "\n\t"
    "mov    r1,  #1  " "\n\t"
    "mov    r2,  #2  " "\n\t"
    "mov    r3,  #3  " "\n\t"
    "mov    r4,  #4  " "\n\t"
    "mov    r5,  #5  " "\n\t"
    "mov    r6,  #6  " "\n\t"
    "mov    r7,  #7  " "\n\t"
    "mov    r8,  #8  " "\n\t"
    "mov    r9,  #9  " "\n\t"
    "mov    r10, #10 " "\n\t"
    "mov    r12, #12 " "\n\t"
    :
    :
    : "cc",
      "r0", "r1", "r2" , "r3" ,
      "r4", "r5", "r6" , "r7" ,
      "r8", "r9", "r10", "r12"
  );
  sys_yield();
  unsigned int n = 17;
  while(1) {
    cputs("In first() loop\n");
    n++;
    first_sub(n);

    __asm__ volatile(
      "mov    r0,  #10 " "\n\t"
      "mov    r1,  #1  " "\n\t"
      "mov    r2,  #2  " "\n\t"
      "mov    r3,  #3  " "\n\t"
      "mov    r4,  #4  " "\n\t"
      "mov    r5,  #5  " "\n\t"
      "mov    r6,  #6  " "\n\t"
      "mov    r7,  #7  " "\n\t"
      "mov    r8,  #8  " "\n\t"
      "mov    r9,  #9  " "\n\t"
      "mov    r10, #10 " "\n\t"
      "mov    r12, #12 " "\n\t"
      :
      :
      : "cc",
        "r0", "r1", "r2" , "r3" ,
        "r4", "r5", "r6" , "r7" ,
        "r8", "r9", "r10", "r12"
    );
    sys_yield();
  }
}

void first_sub(unsigned int arg1) {
  UNUSED(arg1);
  cputs("In first_sub() 1\n");
  sys_yield();
  cputs("In first_sub() 2\n");
 }

/* Second user mode program */
void second(void) {
  cputs("Start second()\n");
    __asm__ volatile(
    "mov    r0,  #10 " "\n\t"
    "mov    r1,  #1  " "\n\t"
    "mov    r2,  #2  " "\n\t"
    "mov    r3,  #3  " "\n\t"
    "mov    r4,  #4  " "\n\t"
    "mov    r5,  #5  " "\n\t"
    "mov    r6,  #6  " "\n\t"
    "mov    r7,  #7  " "\n\t"
    "mov    r8,  #8  " "\n\t"
    "mov    r9,  #9  " "\n\t"
    "mov    r10, #10 " "\n\t"
    "mov    r11, #11 " "\n\t"
    "mov    r12, #12 " "\n\t"
    :
    :
    : "cc",
      "r0", "r1", "r2" , "r3" ,
      "r4", "r5", "r6" , "r7" ,
      "r8", "r9", "r10", "r12"
  );
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

  cputs("     r0 = ");
  cprint_word(thread->registers[0]);
  cputs("     r1 = ");
  cprint_word(thread->registers[1]);
  cputs("     r2 = ");
  cprint_word(thread->registers[2]);
  cputs("     r3 = ");
  cprint_word(thread->registers[3]);
  cputs("\n");

  cputs("     r4 = ");
  cprint_word(thread->registers[4]);
  cputs("     r5 = ");
  cprint_word(thread->registers[5]);
  cputs("     r6 = ");
  cprint_word(thread->registers[6]);
  cputs("     r7 = ");
  cprint_word(thread->registers[7]);
  cputs("\n");

  cputs("     r8 = ");
  cprint_word(thread->registers[8]);
  cputs("     r9 = ");
  cprint_word(thread->registers[9]);
  cputs("    r10 = ");
  cprint_word(thread->registers[10]);
  cputs(" r11/fp = ");
  cprint_word(thread->registers[11]);
  cputs("\n");

  cputs(" r12/ip = ");
  cprint_word(thread->registers[12]);
  cputs(" r13/sp = ");
  cprint_word(thread->registers[13]);
  cputs(" r14/lr = ");
  cprint_word(thread->registers[14]);
  cputs(" r15/pc = ");
  cprint_word(thread->registers[15]);
  cputs("\n");



  cputs("}\n");
}

void *memset(void *b, int c, int len) {
  unsigned char *p = b;
  while(len > 0)
    {
      *p = c;
      p++;
      len--;
    }
  return(b);
}
