.global _start
_start:
    /* Copy interrupt table to install it */
    mov r0, #0x00                /* r0: copy target */
    ldr r1, =interrupt_table     /* r1: copy source */
    ldr r3, =interrupt_table_end /* r3: copy end */
keep_copying_interrupt_table:
    ldr r2, [r1, #0x0]
    str r2, [r0, #0x0]
    add r0, r0, #0x4
    add r1, r1, #0x4
    cmp r1, r3
    bne keep_copying_interrupt_table

    /* Set supervisor stack */
    ldr sp, =0x07ffffff
    msr CPSR_c, #0xD2 /* IRQ mode, interrupts disabled */
    msr CPSR_c, #0xD3 /* Supervisor mode, interrupts disabled */
    bl main

interrupt_table:
    nop
    nop
    ldr pc, svc_entry_address
    nop
    nop
    nop
    ldr pc, irq_entry_address
svc_entry_address:   .word svc_entry
irq_entry_address:   .word irq_entry
interrupt_table_end:
