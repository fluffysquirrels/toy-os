#pragma once

#include "thread.h"

void kernel_init();
void kernel_run();

void scheduler_update_thread_priority(struct thread_t* thread, unsigned int old_priority);
void scheduler_update_thread_state(struct thread_t* thread, unsigned int old_state);
