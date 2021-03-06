#include "asm_constants.h"

.global activate_thread
activate_thread:
    /* Arguments:
         r0: pointer to struct thread_t for the thread to activate
    */

    /* Save kernel registers
       Skip scratch registers r0-r3, ip (r12). */
    mov ip, sp
    push {r4-r11,ip,lr}
    push {r0} /* Save thread_t* from r0 */

    /* Load user state */
    ldr ip, [r0, #THREAD_OFFSET_cpsr]
    msr SPSR, ip
    // r1 = &thread.registers
    add r1, r0, #THREAD_OFFSET_registers

    msr CPSR_c, #(CPSR_MODE_SYS | CPSR_INTERRUPTS_DISABLED)
    ldr sp, [r1, #(4*13)] // restore user mode r13 (sp)
    ldr lr, [r1, #(4*14)] // restore user mode r14 (lr)
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)

    // lr is already saved on the supervisor stack, use it as a
    // register load source
    // lr = &thread.registers
    mov lr, r1
    ldmia lr, {r0-r12}

    // Supervisor lr = user mode r15 (pc)
    ldr lr, [lr, #(4*15)]

    movs pc, lr /* Jump to user mode:
                   returns to lr and restores SPSR into CPSR */

.global svc_entry
svc_entry:
    /* Save user state */

    // push lr so we can calculate a save target in it
    push {lr}
    // lr = &thread
    ldr lr, [sp, #0x4]
    // lr = &thread.registers
    add lr, lr, #THREAD_OFFSET_registers
    stmia lr, {r0-r12}
    // Restore lr
    pop {lr}

    // r0 = &thread
    ldr r0, [sp, #0x0]
    // r1 = &thread.registers
    add r1, r0, #THREAD_OFFSET_registers

    msr CPSR_c, #(CPSR_MODE_SYS | CPSR_INTERRUPTS_DISABLED)
    str sp, [r1, #(4*13)] // save user mode r13 (sp)
    str lr, [r1, #(4*14)] // save user mode r14 (lr)
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)

    str lr, [r1, #(4*15)] // save user mode r15 (pc === supervisor mode's lr)

    mrs ip, SPSR
    str ip, [r0, #THREAD_OFFSET_cpsr]

    /* Load supervisor mode state */
    // Move stack past saved thread_t *thread
    add sp, sp, #0x4
    pop {r4-r11,ip,lr}
    mov sp, ip    /* Restore old sp */

    mov r0, #ACTIVATE_RET_SYSCALL
    bx lr

.global irq_entry
irq_entry:
    /* Save user state */
    // Switch to supervisor mode, which has a stack and thread pointer
    // to store user mode state with.
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)

    // push lr so we can calculate a save target in it
    push {lr}
    // lr = &thread
    ldr lr, [sp, #0x4]
    // lr = &thread.registers
    add lr, lr, #THREAD_OFFSET_registers
    stmia lr, {r0-r12}
    // Restore lr
    pop {lr}

    // r0 = &thread
    ldr r0, [sp, #0x0]
    // r1 = &thread.registers
    add r1, r0, #THREAD_OFFSET_registers

    msr CPSR_c, #(CPSR_MODE_SYS | CPSR_INTERRUPTS_DISABLED)
    str sp, [r1, #(4*13)] // save user mode r13 (sp)
    str lr, [r1, #(4*14)] // save user mode r14 (lr)
    msr CPSR_c, #(CPSR_MODE_IRQ | CPSR_INTERRUPTS_DISABLED)
    sub lr, lr, #0x4 /* lr is address of return address + 1 instruction */
    str lr, [r1, #(4*15)] // save user mode r15 (pc === supervisor mode's lr)

    mrs ip, SPSR
    str ip, [r0, #THREAD_OFFSET_cpsr]

    /* Load supervisor mode state */
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)
    // Move stack past saved thread_t *thread
    add sp, sp, #0x4
    pop {r4-r11,ip,lr}
    mov sp, ip    /* Restore old sp */

    mov r0, #ACTIVATE_RET_IRQ
    bx lr

.global sleep
sleep:
    wfi
    bx lr // Return

.global get_cpsr
get_cpsr:
    // uint32_t get_cpsr();
    // Returns CPSR
    mrs r0, CPSR
    bx lr

.global set_cpsr
set_cpsr:
    // void set_cpsr(uint32_t cpsr);
    msr CPSR, r0
    bx lr

.global get_sctlr
get_sctlr:
    // uint32_t get_sctlr();
    mrc p15, 0, r0, c1, c0, 0
    bx lr

.global get_vbar
get_vbar:
    // uint32_t get_vbar();
    MRC p15, 0, r0, c12, c0, 0
    bx lr

    .global set_vbar
set_vbar:
    // void set_vbar(uint32_t vbar);
    MCR p15, 0, r0, c12, c0, 0
    bx lr
