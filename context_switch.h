#pragma once
#include "stdint.h"
#include "thread.h"

uint32_t activate(struct thread_t *thread);
void sleep(void);
uint32_t get_cpsr();
void set_cpsr(uint32_t cpsr);
