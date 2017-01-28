struct thread_t{
  /* Referenced by offset from assembly, be careful!
     See context_switch.S, asm_constants.h
   */
  unsigned int cpsr;
  unsigned int registers[16];
  /* End of Referenced by offset from assembly */

  unsigned int state;
};

int main(void);
void scheduler_loop(void);
void handle_syscall(struct thread_t*);
void handle_interrupt(struct thread_t*);

void init_thread(struct thread_t *, unsigned int *, unsigned int, unsigned int, void (*)(void));

#define STACK_SIZE 256
#define THREAD_LIMIT 3

// TODO: Access these with more checks via kernel.c functions
extern unsigned int stacks[THREAD_LIMIT][STACK_SIZE];
extern struct thread_t threads[THREAD_LIMIT];
extern unsigned int num_threads;

void scheduler_run(void);

#define UNUSED(x) (void)(x)
