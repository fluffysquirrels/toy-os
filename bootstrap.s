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

    ldr sp, =0x07ffffff
    bl main

interrupt_table:
    ldr pc, dummy_interrupt
    ldr pc, dummy_interrupt
    ldr pc, svc_entry_address
svc_entry_address:   .word svc_entry
dummy_interrupt:     .word 0
interrupt_table_end:
