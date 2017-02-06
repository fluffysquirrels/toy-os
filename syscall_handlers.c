#include "asm_constants.h"
#include "kernel.h"
#include "stdint.h"
#include "stdlib.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "timer.h"
#include "uart.h"
#include "util.h"

#ifndef TRACE_SYSCALL
#define TRACE_SYSCALL 0
#endif

// sysh_t is the typedef of a syscall handler, which are named with the
// prefix 'sysh_'
typedef void (*sysh_t)(struct thread_t*);

static sysh_t syscall_handlers[SYSCALL_NUM_MAX + 1];

void handle_syscall(struct thread_t* thread) {
  unsigned int syscall_num = thread->registers[12];
  sc_LOGF_IF(TRACE_SYSCALL, "syscall_num = %x", syscall_num);

  sysh_t handler = NULL;
  if (syscall_num > SYSCALL_NUM_MAX ||
      (handler = syscall_handlers[syscall_num]) == NULL) {
    sc_LOGF("warn: syscall with unknown syscall_num = %u\n", syscall_num);
    thread_set_uint32_return(thread, E_NOSUCHSYSCALL);
    return;
  }


  sc_LOGF_IF(TRACE_SYSCALL, "calling syscall handler @ %x", (unsigned int) handler);

  handler(thread);

  sc_LOG_IF(TRACE_SYSCALL, "return from syscall handler");
}

static void sysh_yield(struct thread_t* thread) {
  UNUSED(thread);
}

static void sysh_spawn(struct thread_t* thread) {
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

static void sysh_exit(struct thread_t* thread) {
  thread_update_state(thread, THREAD_STATE_EXITED);
  thread = NULL;
}

struct sysh_sleep_callback_state {
  struct thread_t* thread;
};

static void sysh_sleep_callback(struct timer_callback_data_t data, void *cbs_raw);

static void sysh_sleep(struct thread_t* thread) {
  duration_t d = thread_get_uint64_arg(thread, 0);
  sc_LOGF_IF(TRACE_SYSCALL, "d = %llu", d);

  struct sysh_sleep_callback_state *cbs = malloc(sizeof(struct sysh_sleep_callback_state));
  ASSERT(cbs != NULL);
  cbs->thread = thread;

  timer_id_t timer_id = UINT32_MAX;
  err_t err = timer_queue(d, sysh_sleep_callback, cbs, &timer_id);
  if (err != E_SUCCESS) {
    free(cbs);
    thread_set_uint32_return(thread, err);
    return;
  }

  sc_LOGF_IF(TRACE_SYSCALL, "timer_id=%x", timer_id);

  thread_update_state(thread, THREAD_STATE_BLOCKED);
}

static void sysh_sleep_callback(struct timer_callback_data_t data, void *cbs_raw) {
  UNUSED(data);
  struct sysh_sleep_callback_state *cbs = (struct sysh_sleep_callback_state *) cbs_raw;
  thread_update_state(cbs->thread, THREAD_STATE_READY);
  free(cbs);
}

struct file_t *file_get(fd_t fd);
err_t file_begin_read(struct file_t*);

static void sysh_read(struct thread_t* thread) {
  err_t err = E_INVALID;

  struct read_args_t *args = (struct read_args_t *) thread->registers[0];
  struct read_result_t *result = (struct read_result_t *) thread->registers[1];

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

  thread_update_state(thread, THREAD_STATE_BLOCKED);

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

// Called by a driver isr to signal that data is available.
// Current sys_read_callback implementation copies to the user mode buffer during
// an isr. This might be too much work to do in an isr.
void sysh_read_callback(struct sysh_read_callback_state_t *cbs) {
  cbs->file->read_callback_registered = false;

  unsigned int bytes_read = 0;

  while (bytes_read < cbs->args->len) {
    iochar_t ch = file_getch(cbs->file);

    if (ch == EOF) {
      break;
    }

    cbs->args->buff[bytes_read] = (char) ch;
    bytes_read += 1;
  }
  cbs->result->bytes_read = bytes_read;

  cbs->thread->registers[0] = E_SUCCESS;
  thread_update_state(cbs->thread, THREAD_STATE_READY);
  cbs->file->read_callback_registered = false;
  return;
}



void init_syscall_handlers() {
  syscall_handlers[SYSCALL_NUM_YIELD] = &sysh_yield;
  syscall_handlers[SYSCALL_NUM_SPAWN] = &sysh_spawn;
  syscall_handlers[SYSCALL_NUM_EXIT]  = &sysh_exit;
  syscall_handlers[SYSCALL_NUM_READ]  = &sysh_read;
  syscall_handlers[SYSCALL_NUM_SLEEP]  = &sysh_sleep;
}
