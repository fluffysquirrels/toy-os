// Declarations of C system call functions user mode can call.
#pragma once

#include "stdint.h"

typedef unsigned int err_t;
#define E_INVALID       0
#define E_SUCCESS       1
#define E_LIMIT         2
#define E_NOSUCHSYSCALL 3
#define E_NOFILE        4
#define E_BUFFERSIZE    5

typedef int iochar_t;

#ifndef EOF
#define EOF -1
#endif // EOF

// time is a time in nanoseconds since some starting point.
typedef uint64_t time;

// duration_t is a time duration in nanoseconds.
typedef uint64_t duration_t;
#define DURATION_NS ((uint64_t) 1      ) // Nanoseconds
#define DURATION_US (DURATION_NS * 1000) // Microseconds
#define DURATION_MS (DURATION_US * 1000) // Milliseconds
#define DURATION_S  (DURATION_MS * 1000) // Seconds

typedef unsigned int thread_id_t;
typedef unsigned int fd_t;

void sys_yield(void);


// sys_spawn and parameters
struct spawn_args_t {
  void (*pc)(void);
};
struct spawn_result_t {
  thread_id_t thread_id;
};

err_t sys_spawn(struct spawn_args_t *args, struct spawn_result_t *result);

// end sys_spawn

void sys_exit(void);

// sys_read and parameters
struct read_args_t {
  fd_t fd;
  char *buff;
  unsigned int len;
};
struct read_result_t {
  int bytes_read;
};

err_t sys_read(struct read_args_t *args, struct read_result_t *result);
// end sys_read

// Suspend the current thread for at least t.
err_t sys_sleep(duration_t t);

// Calls an invalid system number. For testing error handling.
err_t sys_invalid();
