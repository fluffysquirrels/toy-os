#include "asm_constants.h"

.global activate
activate:
    /* Arguments:
         r0: pointer to struct thread_t for the thread to activate
    */

    /* Save kernel registers
       Skip scratch registers r0-r3, ip (r12). */
    mov ip, sp
    push {r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    push {r0} /* Save thread_t* from r0 */

    /* Load user state */
    ldr ip, [r0, #0x0] /* thread_t.cpsr */
    msr SPSR, ip
    ldr lr, [r0, #0x4] /* thread_t.pc */
    ldr r1, [r0, #0x8] /* thread_t.sp */

    msr CPSR_c, #0xDF /* System mode */
    mov sp, r1
    pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    msr CPSR_c, #0xD3 /* Supervisor mode */

    movs pc, lr /* Jump to user mode:
                   returns to lr and restores SPSR into CPSR */

.global svc_entry
svc_entry:
    // push {r0-r3} to free up some registers
    sub sp, sp, #(4 * 4)
    stmia sp, {r0-r3}

    // r0 = thread_t *thread
    ldr r0, [sp, #(4 * 4)]
    // r1 = &thread->registers
    add r1, r0, #0xc

    // r2 = &thread->registers[4]
    add r2, r1, #(4*4)
    // copy user-mode {r4-r12} to thread->registers[4-12]
    stmia r2, {r4-r12}
    // copy user-mode {r0-r3} from *sp-*(sp + 4 * 4) to registers[0-3] via {r4-r7}
    ldmia sp, {r4-r7}
    stmia r1, {r4-r7}
    // restore supervisor sp to before we pushed {r0-r3}
    add sp, sp, #(4 * 4)

    msr CPSR_c, #0xDF /* System mode */
    str sp, [r1, #(4*13)] // save user-mode r13 (sp)
    str lr, [r1, #(4*14)] // save user-mode r14 (lr)
    msr CPSR_c, #0xD3 /* Supervisor mode */

    str lr, [r1, #(4*15)] // save user mode r15 (pc === supervisor mode's lr)

    /* Save user state */
    msr CPSR_c, #0xDF /* System mode */
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    mov r1, sp  /* store user-mode sp in r1 for use in supervisor mode */
    msr CPSR_c, #0xD3 /* Supervisor mode */

    mrs ip, SPSR
    str ip, [r0, #0x0] /* thread_t.cpsr */
    // TODO: Kill these 2, already in registers
    str lr, [r0, #0x4] /* thread_t.pc */
    str r1, [r0, #0x8] /* thread_t.sp */

    /* Load kernel state */
    // Move stack past saved thread_t *thread
    add sp, sp, #0x4
    pop {r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    mov sp, ip    /* Restore old sp */

    mov r0, #ACTIVATE_RET_SYSCALL
    bx lr

.global irq_entry
irq_entry:
    /* Save user state */
    msr CPSR_c, #0xDF /* System mode */
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    mov r1, sp  /* store user-mode sp in r1 for use later */
    msr CPSR_c, #0xD3 /* Supervisor mode */
    pop {r0} /* r0 = thread_t* used in activate() */
    msr CPSR_c, #0xD2 /* IRQ mode */
    mrs ip, SPSR
    str ip, [r0, #0x0] /* thread_t.cpsr */
    sub lr, lr, #0x4 /* lr is address of return address + 1 instruction */
    str lr, [r0, #0x4] /* thread_t.pc */
    str r1, [r0, #0x8] /* thread_t.sp */

    /* Load kernel state */
    msr CPSR_c, #0xD3 /* Supervisor mode */
    pop {r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    mov sp, ip    /* Restore old sp */

    mov r0, #ACTIVATE_RET_IRQ
    bx lr

.global sleep
sleep:
    wfi
    bx lr
