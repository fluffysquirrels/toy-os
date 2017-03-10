#pragma once

#include "thread.h"

void scheduler_loop(void);
void scheduler_init();

void scheduler_update_thread_priority(struct thread_t* thread, unsigned int old_priority);
void scheduler_update_thread_state(struct thread_t* thread, unsigned int old_state);
