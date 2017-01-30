#include "asm_constants.h"

.global sys_yield
sys_yield:
    mov ip, #SYSCALL_NUM_YIELD
    svc 0
    bx lr

.global sys_spawn
sys_spawn:
    mov ip, #SYSCALL_NUM_SPAWN
    svc 0
    bx lr

.global sys_exit
sys_exit:
    mov ip, #SYSCALL_NUM_EXIT
    svc 0
    bx lr

.global sys_read
sys_read:
    mov ip, #SYSCALL_NUM_READ
    svc 0
    bx lr
