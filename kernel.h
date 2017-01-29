struct thread_t{
  /* Referenced by offset from assembly, be careful!
     See context_switch.S, asm_constants.h
   */
  unsigned int cpsr;
  unsigned int registers[16];
  /* End of Referenced by offset from assembly */

  unsigned int state;
  unsigned int thread_id;
};

#define THREAD_STATE_INVALID 0
#define THREAD_STATE_READY   1
#define THREAD_STATE_EXITED  2

void scheduler_loop(void);
void handle_syscall(struct thread_t*);
void handle_interrupt(struct thread_t*);

#include "syscalls.h"

syscall_error_t kspawn(unsigned int cpsr, void (*pc)(void), struct thread_t **out_thread);

void scheduler_run(void);
