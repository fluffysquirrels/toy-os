#pragma once
#include "limits.h"
#include "stdbool.h"
#include "stdint.h"
#include "syscalls.h"

struct thread_t{
  /* Referenced by offset from assembly, be careful!
     See context_switch.S, asm_constants.h
   */
  unsigned int cpsr;
  unsigned int registers[16];
  /* End of Referenced by offset from assembly */

  // Modify with thread_update_state
  unsigned int state;

  // Do not modify
  unsigned int thread_id;

  // Modify with thread_update_priority
  unsigned int priority;

  uint8_t *stack_base;
  uint32_t stack_size;
};

#define THREAD_STATE_INVALID 0
#define THREAD_STATE_READY   1
#define THREAD_STATE_EXITED  2
#define THREAD_STATE_BLOCKED 3

// Thread priority
// Higher numbers mean higher priority
#define THREAD_PRIORITY_DEFAULT 10
#define THREAD_PRIORITY_MAX 20

err_t kspawn(unsigned int cpsr, void (*pc)(void), struct thread_t **out_thread);
void thread_update_state(struct thread_t *t, unsigned int state);
void thread_update_priority (struct thread_t *t, unsigned int priority);
struct thread_t *thread_get(unsigned int thread_id);
uint64_t thread_get_uint64_arg(struct thread_t* t, unsigned int argument_index);
void thread_set_uint32_return(struct thread_t* t, uint32_t rv);
void sc_print_thread(struct thread_t*);
