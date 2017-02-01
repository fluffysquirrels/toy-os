#include "thread.h"

#include "kernel.h"
#include "stdbool.h"
#include "synchronous_console.h"
#include "util.h"

#define STACK_SIZE 256

static unsigned int stacks[THREAD_LIMIT][STACK_SIZE];

static struct thread_t threads[THREAD_LIMIT];
static unsigned int num_threads = 0;

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
  thread->registers[13] /* sp */ = (unsigned int) (stack_base + STACK_SIZE - 16 /* why -16? */);
  // Set lr to &sys_exit, so when the user mode function returns
  // it will call sys_exit and terminate gracefully.
  thread->registers[14] /* lr */ = (unsigned int) &sys_exit;
  thread->registers[15] /* pc */ = (unsigned int) pc;

  thread_update_priority(thread, THREAD_PRIORITY_DEFAULT);
  thread_update_state(thread, THREAD_STATE_READY);

  if(out_thread != NULL) {
    *out_thread = thread;
  }

  return E_SUCCESS;
}

void thread_update_priority (struct thread_t *t, unsigned int priority) {
  ASSERT(priority <= THREAD_PRIORITY_MAX);

  unsigned int old_priority = t->priority;
  t->priority = priority;

  scheduler_update_thread_priority(t, old_priority);
}

void thread_update_state (struct thread_t *t, unsigned int state) {
  unsigned int old_state = t->state;
  t->state = state;

  // TODO: Remove exited threads from the threads collection, re-use their entries.

  scheduler_update_thread_state(t, old_state);
}

struct thread_t *thread_get(unsigned int thread_id) {
  ASSERT(thread_id < THREAD_LIMIT);
  return &threads[thread_id];
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
