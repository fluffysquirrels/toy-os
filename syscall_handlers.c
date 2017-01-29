#include "asm_constants.h"
#include "kernel.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "util.h"

// sysh_t is the typedef of a syscall handler, which are named with the
// prefix 'sysh_'
typedef void (*sysh_t)(struct thread_t*);

sysh_t syscall_handlers[SYSCALL_NUM_MAX + 1];

void handle_syscall(struct thread_t* thread) {
  unsigned int syscall_num = thread->registers[12];
#if TRACE_SCHEDULER
  sc_puts("handle_syscall() syscall_num = ");
  sc_print_uint32_hex(syscall_num);
  sc_puts("\n");
#endif // TRACE_SCHEDULER

  assert(syscall_num <= SYSCALL_NUM_MAX, "assert failed: syscall_num <= SYSCALL_NUM_MAX");

  sysh_t handler = syscall_handlers[syscall_num];
  if (!handler) {
    warn("handle_syscall() syscall with unknown syscall_num = ");
    sc_puts("  ");
    sc_print_uint32_hex(syscall_num);
    sc_puts("\n");
    return;
  }

#if TRACE_SCHEDULER
  sc_puts("handle_syscall() calling syscall handler @ ");
  sc_print_uint32_hex((unsigned int) handler);
  sc_puts("\n");
#endif // TRACE_SCHEDULER

  handler(thread);

#if TRACE_SCHEDULER
  sc_puts("handle_syscall() returned from syscall handler\n");
#endif // TRACE_SCHEDULER
}

void sysh_yield(struct thread_t* thread) {
  UNUSED(thread);
  sc_puts("handle_syscall() handling yield\n");
}

void sysh_spawn(struct thread_t* thread) {
  struct spawn_args_t *pargs = (struct spawn_args_t *) thread->registers[0];
  struct spawn_result_t *presult = (struct spawn_result_t *) thread->registers[1];

  struct thread_t* spawned_thread = NULL;
  syscall_error_t err = SE_INVALID;

  err = kspawn(0x10, pargs->pc, &spawned_thread);
  if(err != SE_SUCCESS) {
    thread->registers[0] = err;
    return;
  }

  presult->thread_id = spawned_thread->thread_id;
  err = SE_SUCCESS;
  thread->registers[0] = err;
  return;
}

void sysh_exit(struct thread_t* thread) {
  thread->state = THREAD_STATE_EXITED;
  // TODO: Remove thread from the threads collection, re-use its entry.
}

void init_syscall_handlers() {
  syscall_handlers[SYSCALL_NUM_YIELD] = &sysh_yield;
  syscall_handlers[SYSCALL_NUM_SPAWN] = &sysh_spawn;
  syscall_handlers[SYSCALL_NUM_EXIT]  = &sysh_exit;
}
