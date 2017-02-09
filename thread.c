#include "thread.h"

#include "kernel.h"
#include "stdbool.h"
#include "stdlib.h"
#include "synchronous_console.h"
#include "third_party/OpenBSD_collections/src/tree.h"
#include "util.h"

#ifndef TRACE_THREAD
#define TRACE_THREAD 0
#endif // TRACE_THREAD

#define THREAD_LIMIT 100
#define STACK_SIZE 1024

struct thread_node {
  RB_ENTRY(thread_node) tree_node;
  uint8_t *stack_base;
  uint32_t stack_size;
  struct thread_t thread;
};

DEFINE_KEY_COMPARER(thread_node_cmp, struct thread_node, thread_id_t, thread.thread_id)

static RB_HEAD(thread_map, thread_node) threads = RB_INITIALIZER(&threads);
RB_GENERATE(thread_map, thread_node, tree_node, thread_node_cmp)

static struct thread_node *thread_node_find(unsigned int thread_id);
static void thread_delete (struct thread_t *t);

static unsigned int num_threads = 0;
static thread_id_t largest_thread_id = 0;

err_t kspawn(unsigned int cpsr, void (*pc)(void), struct thread_t **out_thread) {

  sc_LOGF_IF(TRACE_THREAD, "cpsr = %x, pc = %x", cpsr, (uint32_t) pc);

  if (num_threads >= THREAD_LIMIT) {
    return E_LIMIT;
  }

  struct thread_node *existing;
  do {
    largest_thread_id++;
    existing = thread_node_find(largest_thread_id);
  } while (existing != NULL);

  thread_id_t thread_id = largest_thread_id;
  num_threads++;

  struct thread_node *thread_node = malloc(sizeof(struct thread_node));
  ASSERT(thread_node != NULL);
  memset(thread_node, 0, sizeof(struct thread_node));
  struct thread_t *thread = &thread_node->thread;
  thread->thread_id = thread_id;
  RB_INSERT(thread_map, &threads, thread_node);

  thread->stack_size = STACK_SIZE;
  thread->stack_base = malloc(thread->stack_size);
  ASSERT(thread->stack_base != NULL);
  memset(thread->stack_base, 0, thread->stack_size);
  // initial_stack is the last word of the stack.
  uint8_t *initial_stack = thread->stack_base + thread->stack_size - 4;

  thread->cpsr = cpsr;
  thread->registers[13] /* sp */ = (unsigned int) initial_stack;
  // Set lr to &sys_exit, so when the user mode function returns
  // it will call sys_exit and terminate gracefully.
  thread->registers[14] /* lr */ = (unsigned int) &sys_exit;
  thread->registers[15] /* pc */ = (unsigned int) pc;

  thread_update_priority(thread, THREAD_PRIORITY_DEFAULT);
  thread_update_state(thread, THREAD_STATE_READY);

  if(out_thread != NULL) {
    *out_thread = thread;
  }

  sc_LOGF_IF(TRACE_THREAD, "&thread = %x, thread_id = %x", thread, thread->thread_id);

  return E_SUCCESS;
}

void thread_update_priority (struct thread_t *t, unsigned int priority) {
  ASSERT(priority <= THREAD_PRIORITY_MAX);

  unsigned int old_priority = t->priority;
  t->priority = priority;

  sc_LOGF_IF(TRACE_THREAD,
    "\n"
    "  thread_id    = %x\n"
    "  priority     = %x\n"
    "  old_priority = %x",
    t->thread_id, priority, old_priority);

  scheduler_update_thread_priority(t, old_priority);
}

void thread_update_state (struct thread_t *t, unsigned int state) {
  unsigned int old_state = t->state;
  t->state = state;

  scheduler_update_thread_state(t, old_state);

  sc_LOGF_IF(TRACE_THREAD,
    "\n"
    "  thread_id    = %x\n"
    "  state        = %x\n"
    "  old_state    = %x",
    t->thread_id, state, old_state);

  if (state == THREAD_STATE_EXITED) {
    thread_delete(t);
    t = NULL;
  }
}

static void thread_delete (struct thread_t *t) {
  sc_LOGF_IF(TRACE_THREAD, "thread_id = ", t->thread_id);

  struct thread_node *node = thread_node_find(t->thread_id);
  ASSERT(node != NULL);
  RB_REMOVE(thread_map, &threads, node);
  free(node->thread.stack_base);
  node->thread.stack_base = NULL;
  free(node);
  node = NULL;
  num_threads--;
}

static struct thread_node *thread_node_find(unsigned int thread_id) {
  struct thread_node query = {
    .thread = {
      .thread_id = thread_id,
    },
  };
  return RB_FIND(thread_map, &threads, &query);
}

struct thread_t *thread_get(unsigned int thread_id) {
  struct thread_node *tn = thread_node_find(thread_id);
  return tn == NULL ? NULL : &tn->thread;
}

uint64_t thread_get_uint64_arg(struct thread_t* t, unsigned int argument_index) {
  uint64_t low  = (uint64_t) t->registers[argument_index + 0];
  uint64_t high = (uint64_t) t->registers[argument_index + 1];
  return (high << 32) | low;
}

void thread_set_uint32_return(struct thread_t* t, uint32_t rv) {
  t->registers[0] = rv;
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
  sc_printf(" .priority = %x\n", thread->priority);

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
