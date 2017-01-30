#include "asm_constants.h"
#include "kernel.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "uart.h"
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
    thread->registers[0] = E_NOSUCHSYSCALL;
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
  err_t err = E_INVALID;

  err = kspawn(0x10, pargs->pc, &spawned_thread);
  if(err != E_SUCCESS) {
    thread->registers[0] = err;
    return;
  }

  presult->thread_id = spawned_thread->thread_id;
  err = E_SUCCESS;
  thread->registers[0] = err;
  return;
}

void sysh_exit(struct thread_t* thread) {
  thread->state = THREAD_STATE_EXITED;
  // TODO: Remove thread from the threads collection, re-use its entry.
}

struct file_t *file_get(fd_t fd);
err_t file_begin_read(struct file_t*);


void sysh_read(struct thread_t* thread) {
  err_t err = E_INVALID;
  UNUSED(err);
  struct read_args_t *args = (struct read_args_t *) thread->registers[0];
  struct read_result_t *result = (struct read_result_t *) thread->registers[1];

  if (args->len <= 0) {
    thread->registers[0] = E_BUFFERSIZE;
    return;
  }

  struct file_t *file = file_get(args->fd);
  if (file == NULL) {
    thread->registers[0] = E_NOFILE;
    return;
  }

  if (file->read_callback_registered) {
    thread->registers[0] = E_LIMIT;
    return;
  }

  err = file_begin_read(file);
  if (err != E_SUCCESS) {
    thread->registers[0] = err;
    return;
  }
  struct sysh_read_callback_state_t cbs = {
    .args = args,
    .file = file,
    .result = result,
    .thread = thread,
  };
  file->read_callback_registered = true;
  file->read_callback_state = cbs;

  thread->state = THREAD_STATE_BLOCKED;
  return;
}

struct file_t uart_0_file = {
  .fd = 17,
  .read_callback_registered = false,
};

struct file_t* file_get(fd_t fd) {
  UNUSED(fd);
  // TODO: Choose a file based on the fd.
  return &uart_0_file;
}

err_t file_begin_read(struct file_t* file) {
  UNUSED(file);
  // TODO: Choose a device based on the file.
  return uart_begin_read(uart_0);
}

iochar_t file_getch(struct file_t* file) {
  UNUSED(file);
  // TODO: Choose a device based on the file.
  return uart_getch(uart_0);
}

void sysh_read_callback(struct sysh_read_callback_state_t *cbs) {
  cbs->file->read_callback_registered = false;

  // TODO: Read multiple bytes.
  iochar_t ch = file_getch(cbs->file);
  unsigned int bytes_read = 0;
  if (ch != EOF) {
    bytes_read = 1;
    assert(cbs->args->len >= 1, "failed assert cbs->args->len >= 1");
    cbs->args->buff[0] = (char) ch;
  }
  cbs->result->bytes_read = bytes_read;

  cbs->thread->registers[0] = E_SUCCESS;
  cbs->thread->state = THREAD_STATE_READY;
  cbs->file->read_callback_registered = false;
  return;
}

void init_syscall_handlers() {
  syscall_handlers[SYSCALL_NUM_YIELD] = &sysh_yield;
  syscall_handlers[SYSCALL_NUM_SPAWN] = &sysh_spawn;
  syscall_handlers[SYSCALL_NUM_EXIT]  = &sysh_exit;
  syscall_handlers[SYSCALL_NUM_READ]  = &sysh_read;
}
