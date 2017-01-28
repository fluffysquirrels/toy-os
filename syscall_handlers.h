typedef void (*syscall_t)(struct thread_t*);

void handle_syscall(struct thread_t*);
void set_syscall_handlers();
