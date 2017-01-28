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

void enable_timer0_interrupt(void);
void start_periodic_timer0(void);

void first(void);
void first_sub(unsigned int);
void second(void);

void sc_print_thread(struct thread_t*);

void *memset(void*, int, int);

#define UNUSED(x) (void)(x)

#define STACK_SIZE 256
#define THREAD_LIMIT 2

// TODO: Access these with more checks via kernel.c functions
extern unsigned int stacks[THREAD_LIMIT][STACK_SIZE];
extern struct thread_t threads[THREAD_LIMIT];
extern unsigned int num_threads;
