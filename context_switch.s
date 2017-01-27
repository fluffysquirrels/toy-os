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
    /* Save user state */
    msr CPSR_c, #0xDF /* System mode */
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    mov r1, sp  /* store user-mode sp in r1 for use in supervisor mode */
    msr CPSR_c, #0xD3 /* Supervisor mode */

    pop {r0} /* r0 = thread_t* used in activate() */
    mrs ip, SPSR
    str ip, [r0, #0x0] /* thread_t.cpsr */
    str lr, [r0, #0x4] /* thread_t.pc */
    str r1, [r0, #0x8] /* thread_t.sp */

    /* Load kernel state */
    pop {r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}
    mov sp, ip    /* Restore old sp */

    mov r0, #0x2     /* Return 0x2 = execution stopped by syscall */
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

    mov r0, #0x1     /* Return 0x1 = execution stopped by interrupt */
    bx lr

.global sleep
sleep:
    wfi
    bx lr
