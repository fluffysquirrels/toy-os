// Declarations of C system call functions user mode can call.

#pragma once

typedef unsigned int syscall_error_t;
#define SE_INVALID 0
#define SE_SUCCESS 1
#define SE_LIMIT 2

void sys_yield(void);

typedef unsigned int thread_id_t;
struct spawn_args_t {
  void (*pc)(void);
};
struct spawn_result_t {
  thread_id_t thread_id;
};
syscall_error_t sys_spawn(struct spawn_args_t *args, struct spawn_result_t *result);

void sys_exit(void);
