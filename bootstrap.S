#include "asm_constants.h"

.global _start
.section .text.startup
_start:
    // Set initial stacks, 2^16 = 64KB stack per mode
    msr CPSR_c, #(CPSR_MODE_USR | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =0x07fffff0
    msr CPSR_c, #(CPSR_MODE_FIQ | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =0x07fefff0
    msr CPSR_c, #(CPSR_MODE_IRQ | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =0x07fdfff0
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =0x07fcfff0
    // Skip CPSR_MODE_MON, as we won't be using secure extensions.
    msr CPSR_c, #(CPSR_MODE_ABT | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =0x07fbfff0
    // Skip CPSR_MODE_HYP, as we won't be using virtualisation extensions.
    msr CPSR_c, #(CPSR_MODE_UND | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =0x07fafff0
    // Skip CPSR_MODE_SYS, which has same sp as USR mode.

    // Start in supervisor mode
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)

    // Zero out .bss section
    // We will call  void *memset(void *bytes, int ch, int len)
    ldr r0, =__bss_start__   // void *bytes
    mov r1, #0x0             // int ch
    ldr r12, =__bss_end__
    sub r2, r12, r0          // int len
    bl memset

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

    bl main

interrupt_table:
    nop
    nop
    ldr pc, svc_entry_address
    nop
    nop
    nop
    ldr pc, irq_entry_address
    nop
svc_entry_address:   .word svc_entry
irq_entry_address:   .word irq_entry
interrupt_table_end:
