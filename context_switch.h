#pragma once

#include "stdint.h"
#include "thread.h"

uint32_t activate_thread(struct thread_t *thread);
void sleep(void);
uint32_t get_cpsr();
void set_cpsr(uint32_t cpsr);
uint32_t get_sctlr();
uint32_t get_vbar();
void set_vbar(uint32_t vbar);
