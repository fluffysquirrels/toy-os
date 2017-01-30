// Declarations of C system call functions user mode can call.

#pragma once

typedef unsigned int err_t;
#define E_INVALID       0
#define E_SUCCESS       1
#define E_LIMIT         2
#define E_NOSUCHSYSCALL 3
#define E_NOFILE        4
#define E_BUFFERSIZE    5

typedef int iochar_t;
#define EOF -1

void sys_yield(void);

typedef unsigned int thread_id_t;
typedef unsigned int fd_t;

struct spawn_args_t {
  void (*pc)(void);
};
struct spawn_result_t {
  thread_id_t thread_id;
};

err_t sys_spawn(struct spawn_args_t *args, struct spawn_result_t *result);

void sys_exit(void);



struct read_args_t {
  fd_t fd;
  char *buff;
  unsigned int len;
};
struct read_result_t {
  int bytes_read;
};

err_t sys_read(struct read_args_t *args, struct read_result_t *result);
