#include "asm_constants.h"

#define SYSCALL_STUB(name, num) \
  .global name                 ;\
  name:                        ;\
      mov ip, #(num)           ;\
      svc 0                    ;\
      bx lr                    ;

SYSCALL_STUB(sys_yield, SYSCALL_NUM_YIELD)
SYSCALL_STUB(sys_spawn, SYSCALL_NUM_SPAWN)
SYSCALL_STUB(sys_exit,  SYSCALL_NUM_EXIT)
SYSCALL_STUB(sys_read, SYSCALL_NUM_READ)
SYSCALL_STUB(sys_sleep, SYSCALL_NUM_SLEEP)
SYSCALL_STUB(sys_invalid, SYSCALL_NUM_MAX + 2)
