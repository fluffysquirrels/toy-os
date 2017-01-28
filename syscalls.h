// Declarations of C system call functions user mode can call.

typedef unsigned int syscall_return_t;

void sys_yield(void);
// TODO: Update signature with arg struct, result struct, error code.

typedef unsigned int thread_id_t;
struct spawn_args_t {
  void (*pc)(void);
};
struct spawn_result_t {
  thread_id_t thread_id;
};
syscall_return_t sys_spawn(struct spawn_args_t *args, struct spawn_result_t *result);
