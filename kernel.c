#include "asm_constants.h"
#include "context_switch.h"
#include "kernel.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "uart.h"
#include "util.h"
#include "versatilepb.h"

void scheduler_init(void);
void scheduler_loop(void);

void handle_interrupt(struct thread_t*);
void init_timers(void);
void start_scheduler_timer(void);

void sc_print_thread(struct thread_t*);

void pic_log_status();

#define TRACE_SCHEDULER 0
#define TRACE_INTERRUPTS 0

#define STACK_SIZE 256
#define THREAD_LIMIT 4

unsigned int stacks[THREAD_LIMIT][STACK_SIZE];
struct thread_t threads[THREAD_LIMIT];
unsigned int num_threads = 0;

isr_t interrupt_handlers[PIC_INTNUM_COUNT];

err_t kspawn(unsigned int cpsr, void (*pc)(void), struct thread_t **out_thread) {

  if (num_threads >= THREAD_LIMIT) {
    return E_LIMIT;
  }

  unsigned int thread_idx = num_threads;
  num_threads++;
  struct thread_t *thread = &threads[thread_idx];

  unsigned int *stack_base = stacks[thread_idx];

  thread->cpsr = cpsr;
  thread->thread_id = thread_idx;
  thread->state = THREAD_STATE_READY;
  thread->registers[13] /* sp */ = (unsigned int) (stack_base + STACK_SIZE - 16 /* why -16? */);
  // Set lr to &sys_exit, so when the user mode function returns
  // it will call sys_exit and terminate gracefully.
  thread->registers[14] /* lr */ = (unsigned int) &sys_exit;
  thread->registers[15] /* pc */ = (unsigned int) pc;

  if(out_thread != NULL) {
    *out_thread = thread;
  }

  return E_SUCCESS;
}

void scheduler_run() {
  scheduler_init();
  scheduler_loop();
}

void scheduler_init() {
  init_syscall_handlers();
  uart_init();
  init_timers();
  start_scheduler_timer();
}

void scheduler_loop() {
#if TRACE_SCHEDULER
    sc_puts("\nscheduler_loop() start\n");
#endif // TRACE_SCHEDULER

  unsigned int thread_idx = 0;

  while(1) {
#if TRACE_SCHEDULER
      sc_puts("\nscheduler_loop()\n");
#endif // TRACE_SCHEDULER

    unsigned int pic_irqstatus = *(PIC + VIC_IRQSTATUS);
    bool interrupt_active = pic_irqstatus != 0;
    if(interrupt_active) {
      handle_interrupt(NULL);
      continue;
    }

    bool no_threads_ready = true;
    for (unsigned int ix = 0; ix < num_threads; ix++) {
      if (threads[ix].state == THREAD_STATE_READY) {
        no_threads_ready = false;
        break;
      }
    }

    if (no_threads_ready) {
#if TRACE_SCHEDULER
      sc_puts("scheduler_loop() no threads ready, sleeping\n\n");
#endif // TRACE_SCHEDULER

      sleep();
      continue;
    }

    struct thread_t *thread = &threads[thread_idx];

#if TRACE_SCHEDULER
    sc_puts("scheduler_loop() thread_idx=");
    sc_print_uint32_hex(thread_idx);
    sc_puts("\n");
    sc_print_thread(thread);
#endif // TRACE_SCHEDULER

    /* Can't use % to calculate new thread_idx because that requires a
       runtime library function */
    thread_idx++;
    if(thread_idx >= num_threads) {
      thread_idx = 0;
    }

    if (thread->state != THREAD_STATE_READY) {
#if TRACE_SCHEDULER
      sc_puts("scheduler_loop() skipping thread that is not ready\n");
#endif // TRACE_SCHEDULER

      continue;
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

#if TRACE_INTERRUPTS
  sc_puts("\nhandle_interrupt()\n");
#endif // TRACE_SCHEDULER

  unsigned int pic_irqstatus = *(PIC + VIC_IRQSTATUS);

#if TRACE_INTERRUPTS
  pic_log_status();
#endif // TRACE_INTERRUPTS

  if(pic_irqstatus == 0) {
    panic("handle_interrupt() no interrupt active?");
    return;
  }

  int irq = __builtin_ctz(pic_irqstatus);

#if TRACE_INTERRUPTS
  sc_puts("handle_interrupt() irq = ");
  sc_print_uint32_hex(irq);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  assert(irq >= 0, "assert failed: irq >= 0");
  assert(irq < PIC_INTNUM_COUNT, "assert failed: irq <= PIC_INTNUM_COUNT");

  isr_t isr = interrupt_handlers[irq];

  if(!isr) {
    sc_puts("irq = ");
    sc_print_uint32_hex(irq);
    sc_puts("\n");
    panic("Interrupt with no handler");
    // Not reached.
    return;
  }

#if TRACE_INTERRUPTS
  sc_puts("handle_interrupt() calling handler @");
  sc_print_uint32_hex((unsigned int) isr);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  isr();

#if TRACE_INTERRUPTS
  sc_puts("handle_interrupt() returned from handler\n");
  pic_log_status();
#endif // TRACE_INTERRUPTS
}

void pic_log_status() {
  sc_puts("pic_log_status()\n");
  sc_puts("  PIC_IRQSTATUS = ");
  sc_print_uint32_hex(*(PIC + VIC_IRQSTATUS));
  sc_puts("\n");
  sc_puts("  PIC_RAWINTR = ");
  sc_print_uint32_hex(*(PIC + VIC_RAWINTR));
  sc_puts("\n");
  sc_puts("  PIC_INTENABLE = ");
  sc_print_uint32_hex(*(PIC + VIC_INTENABLE));
  sc_puts("\n");
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

void set_interrupt_handler(unsigned char irq, isr_t isr) {
#if TRACE_INTERRUPTS
  sc_puts("set_interrupt_handler() irq = ");
  sc_print_uint32_hex(irq);
  sc_puts(" isr @ ");
  sc_print_uint32_hex((unsigned int) isr);
  sc_puts("\n");
#endif // TRACE_INTERRUPTS

  assert(interrupt_handlers[irq] == NULL, "failed assert interrupt_handlers[irq] == NULL");
  interrupt_handlers[irq] = isr;
}

void init_timers() {
  set_interrupt_handler(PIC_INTNUM_TIMER01, isr_timer01);

  // Enable interrupt
  *(PIC + VIC_INTENABLE) |= PIC_INTMASK_TIMER01;
}

void start_scheduler_timer() {
  *(TIMER0 + TIMER_LOAD) = 500 * 1000;
  *(TIMER0 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
  *(TIMER0 + TIMER_BGLOAD) = 500 * 1000;

//  *(TIMER1 + TIMER_LOAD) = 600000;
//  *(TIMER1 + TIMER_CONTROL) = TIMER_EN | TIMER_PERIODIC | TIMER_32BIT | TIMER_INTEN;
//  *(TIMER1 + TIMER_BGLOAD) = 600000;
}

void sc_print_thread(struct thread_t *thread) {
  sc_puts("thread {\n");
  sc_puts("  .cpsr = ");
  sc_print_uint32_hex(thread->cpsr);
  sc_puts(" .state = ");
  sc_print_uint32_hex(thread->state);
  sc_puts(" .thread_id = ");
  sc_print_uint32_hex(thread->thread_id);
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
