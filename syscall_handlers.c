#include "asm_constants.h"
#include "kernel.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"

syscall_t syscall_handlers[SYSCALL_NUM_MAX + 1];

void handle_syscall(struct thread_t* thread) {
  unsigned int syscall_num = thread->registers[12];
#if TRACE_SCHEDULER
  sc_puts("handle_syscall() syscall_num = ");
  sc_print_uint32_hex(syscall_num);
  sc_puts("\n");
#endif // TRACE_SCHEDULER

  assert(syscall_num <= SYSCALL_NUM_MAX, "assert failed: syscall_num <= SYSCALL_NUM_MAX");

  syscall_t handler = syscall_handlers[syscall_num];
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

void syscall_handler_yield(struct thread_t* thread) {
  UNUSED(thread);
  sc_puts("handle_syscall() handling yield\n");
}

void syscall_handler_spawn(struct thread_t* thread) {
  warn("spawn not yet implemented");

  struct spawn_args_t *pargs = (struct spawn_args_t *) thread->registers[0];
  struct spawn_result_t *presult = (struct spawn_result_t *) thread->registers[1];

  UNUSED(pargs);

  struct thread_t* spawned_thread = NULL;
  syscall_error_t err = SE_INVALID;

  err = kspawn(0x10, pargs->pc, &spawned_thread);
  if(err != SE_SUCCESS) {
    thread->registers[0] = err;
    return;
  }

  presult->thread_id = 91;
  err = SE_SUCCESS;
  thread->registers[0] = err;
  return;
}

void set_syscall_handlers() {
  syscall_handlers[SYSCALL_NUM_YIELD] = &syscall_handler_yield;
  syscall_handlers[SYSCALL_NUM_SPAWN] = &syscall_handler_spawn;
}
