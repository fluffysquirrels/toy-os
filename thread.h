#pragma once
#include "stdbool.h"
#include "syscalls.h"

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
#define THREAD_STATE_BLOCKED 3

err_t kspawn(unsigned int cpsr, void (*pc)(void), struct thread_t **out_thread);
bool no_threads_ready();
void sc_print_thread(struct thread_t*);

// TODO: Hide these behind an abstraction
#define THREAD_LIMIT 4
