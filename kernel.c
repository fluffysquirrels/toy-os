#include "asm_constants.h"
#include "context_switch.h"
#include "kernel.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "versatilepb.h"

void handle_syscall(struct thread_t*);
void handle_interrupt(struct thread_t*);

void scheduler_init(void);
void scheduler_loop(void);

void set_interrupt_handlers();
void enable_timer01_interrupt(void);
void start_scheduler_timer(void);

void sc_print_thread(struct thread_t*);

void *memset(void*, int, int);

#define TRACE_SCHEDULER 1

unsigned int stacks[THREAD_LIMIT][STACK_SIZE];
struct thread_t threads[THREAD_LIMIT];
unsigned int num_threads = 0;

typedef void (*isr_t)(void);
isr_t interrupt_handlers[PIC_INTNUM_COUNT];

void init_thread(struct thread_t *out_thread, unsigned int *stack_base, unsigned int stack_size, unsigned int cpsr, void (*pc)(void)) {
  out_thread->cpsr = cpsr;
  memset(out_thread->registers, '\0', sizeof(out_thread->registers));
  out_thread->registers[13] = (unsigned int) (stack_base + stack_size - 16 /* why -16? */);
  out_thread->registers[15] = (unsigned int) pc;
}

void scheduler_run() {
  scheduler_init();
  scheduler_loop();
}

void scheduler_init() {
  set_interrupt_handlers();
  enable_timer01_interrupt();
  start_scheduler_timer();
}

void scheduler_loop() {
#if TRACE_SCHEDULER
    sc_puts("\nscheduler_loop() start\n");
#endif // TRACE_SCHEDULER

  unsigned int thread_idx = 0;

  while(1) {
    struct thread_t *thread = &threads[thread_idx];
    /* Can't use % to calculate new thread_idx because that requires a
       runtime library function */

#if TRACE_SCHEDULER
    sc_puts("\nscheduler_loop() thread_idx=");
    sc_print_uint32_hex(thread_idx);
    sc_puts("\n");
    sc_print_thread(thread);
#endif // TRACE_SCHEDULER

    thread_idx++;
    if(thread_idx >= num_threads) {
      thread_idx = 0;
    }

    unsigned int stop_reason = activate(thread);

#if TRACE_SCHEDULER
    sc_puts("scheduler_loop() activate returned ");
    sc_print_uint32_hex(stop_reason);
    if(stop_reason == ACTIVATE_RET_IRQ) {
      sc_puts(" = ACTIVATE_RET_IRQ");
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      sc_puts(" = ACTIVATE_RET_SYSCALL");
    }
    sc_puts("\n");
#endif // TRACE_SCHEDULER

    if(stop_reason == ACTIVATE_RET_IRQ) {
      handle_interrupt(thread);
    } else if (stop_reason == ACTIVATE_RET_SYSCALL) {
      handle_syscall(thread);
    }
  }

  /* Not reached */
}

void handle_interrupt(struct thread_t* thread) {
  UNUSED(thread);

#if TRACE_SCHEDULER
  sc_puts("handle_interrupt()\n");
#endif // TRACE_SCHEDULER

  unsigned int pic_irqstatus = *(PIC + VIC_IRQSTATUS);

#if TRACE_SCHEDULER
  sc_puts("handle_interrupt() pic_irqstatus = ");
  sc_print_uint32_hex(pic_irqstatus);
  sc_puts("\n");
#endif

  if(pic_irqstatus == 0) {
    panic("handle_interrupt() no interrupt active?");
    return;
  }

  int irq = __builtin_ctz(pic_irqstatus);

#if TRACE_SCHEDULER
  sc_puts("handle_interrupt() irq = ");
  sc_print_uint32_hex(irq);
  sc_puts("\n");
#endif // TRACE_SCHEDULER

  assert(irq >= 0, "irq >= 0");
  assert(irq < PIC_INTNUM_COUNT, "irq <= PIC_INTNUM_COUNT");

  isr_t isr = interrupt_handlers[irq];

  if(!isr) {
    warn("Interrupt with no isr");
    sc_puts("irq = ");
    sc_print_uint32_hex(irq);
    sc_puts("\n");
    panic("Interrupt with no isr");
    // Not reached.
    return;
  }

#if TRACE_SCHEDULER
  sc_puts("handle_interrupt() running isr\n");
#endif // TRACE_SCHEDULER

  isr();

#if TRACE_SCHEDULER
  sc_puts("handle_interrupt() back from isr\n");
#endif // TRACE_SCHEDULER
}

void handle_syscall(struct thread_t* thread) {
  unsigned int syscall_num = thread->registers[12];
#if TRACE_SCHEDULER
  sc_puts("handle_syscall() syscall_num = ");
  sc_print_uint32_hex(syscall_num);
  sc_puts("\n");
#endif // TRACE_SCHEDULER
  switch(syscall_num) {
  case SYSCALL_NUM_YIELD:
#if TRACE_SCHEDULER
    sc_puts("handle_syscall() handling yield\n");
#endif // TRACE_SCHEDULER
    break;
  default:
    warn("handle_syscall() syscall with unknown syscall_num = ");
    sc_puts("  ");
    sc_print_uint32_hex(syscall_num);
    sc_puts("\n");
  }
}

void isr_timer01() {
#if TRACE_SCHEDULER
    sc_puts("isr_timer01()\n");
#endif // TRACE_SCHEDULER

  if(*(TIMER0 + TIMER_MIS)) { /* Timer0 went off */
    *(TIMER0 + TIMER_INTCLR) = 1; /* Clear interrupt */
#if TRACE_SCHEDULER
    sc_puts("isr_timer01() TIMER0 tick\n");
#endif // TRACE_SCHEDULER
  }  else if(*(TIMER1 + TIMER_MIS)) { /* Timer1 went off */
    *(TIMER1 + TIMER_INTCLR) = 1; /* Clear interrupt */
#if TRACE_SCHEDULER
    sc_puts("isr_timer01() TIMER1 tick\n");
#endif // TRACE_SCHEDULER
  } else {
    panic("isr_timer01() *(TIMER0/1 + TIMER_MIS) was clear");
  }
}

void set_interrupt_handlers() {
  interrupt_handlers[PIC_INTNUM_TIMER01] = &isr_timer01;
}

void enable_timer01_interrupt() {
  *(PIC + VIC_INTENABLE) |= PIC_INTMASK_TIMER01;
}

void start_scheduler_timer() {
  *(TIMER0 + TIMER_LOAD) = 1000000;
  *(TIMER0 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
  *(TIMER0 + TIMER_BGLOAD) = 1000000;

  *(TIMER1 + TIMER_LOAD) = 600000;
  *(TIMER1 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
  *(TIMER1 + TIMER_BGLOAD) = 600000;

}

void sc_print_thread(struct thread_t *thread) {
  sc_puts("thread {\n");
  sc_puts("  .cpsr = ");
  sc_print_uint32_hex(thread->cpsr);
  sc_puts("\n");

  sc_puts("     r0 = ");
  sc_print_uint32_hex(thread->registers[0]);
  sc_puts("     r1 = ");
  sc_print_uint32_hex(thread->registers[1]);
  sc_puts("     r2 = ");
  sc_print_uint32_hex(thread->registers[2]);
  sc_puts("     r3 = ");
  sc_print_uint32_hex(thread->registers[3]);
  sc_puts("\n");

  sc_puts("     r4 = ");
  sc_print_uint32_hex(thread->registers[4]);
  sc_puts("     r5 = ");
  sc_print_uint32_hex(thread->registers[5]);
  sc_puts("     r6 = ");
  sc_print_uint32_hex(thread->registers[6]);
  sc_puts("     r7 = ");
  sc_print_uint32_hex(thread->registers[7]);
  sc_puts("\n");

  sc_puts("     r8 = ");
  sc_print_uint32_hex(thread->registers[8]);
  sc_puts("     r9 = ");
  sc_print_uint32_hex(thread->registers[9]);
  sc_puts("    r10 = ");
  sc_print_uint32_hex(thread->registers[10]);
  sc_puts(" r11/fp = ");
  sc_print_uint32_hex(thread->registers[11]);
  sc_puts("\n");

  sc_puts(" r12/ip = ");
  sc_print_uint32_hex(thread->registers[12]);
  sc_puts(" r13/sp = ");
  sc_print_uint32_hex(thread->registers[13]);
  sc_puts(" r14/lr = ");
  sc_print_uint32_hex(thread->registers[14]);
  sc_puts(" r15/pc = ");
  sc_print_uint32_hex(thread->registers[15]);
  sc_puts("\n");

  sc_puts("}\n");
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
