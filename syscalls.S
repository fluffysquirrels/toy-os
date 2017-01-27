#include "asm_constants.h"

.global sys_yield
sys_yield:
    mov ip, #SYSCALL_NUM_YIELD
    svc 0
    bx lr
