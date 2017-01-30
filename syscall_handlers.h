void handle_syscall(struct thread_t*);
void init_syscall_handlers();

struct file_t;

struct sysh_read_callback_state_t {
  struct read_args_t *args;
  struct read_result_t *result;
  struct thread_t* thread;
  struct file_t* file;
};

struct file_t {
  fd_t fd;
  bool read_callback_registered;
  struct sysh_read_callback_state_t read_callback_state;
};

void sysh_read_callback(struct sysh_read_callback_state_t *cbs);
