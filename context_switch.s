.global activate
activate:
    /* Arguments:
         r0: pointer to struct thread_t for the thread to activate
    */

    /* Save kernel registers
       Skip scratch registers r1-r3, ip (r12).
       Keep thread_t argument in r0 */
    mov ip, sp
    push {r0,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}

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

    mov r2, lr /* store syscall lr == thread's pc in r2 */

    /* Load kernel state
       Current thread_t is in r0 */
    pop {r0,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}

    mov r3, ip    /* Old sp */
    mrs ip, SPSR
    str ip, [r0, #0x0] /* thread_t.cpsr */
    str r2, [r0, #0x4] /* thread_t.pc */
    str r1, [r0, #0x8] /* thread_t.sp */

    mov sp, r3
    bx lr

.global halt
halt:
    wfi
    bx lr
