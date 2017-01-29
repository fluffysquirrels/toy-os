typedef void (*syscall_t)(struct thread_t*);

void handle_syscall(struct thread_t*);
void init_syscall_handlers();
