#include "asm_constants.h"

.global _start
.section .text.startup
_start:
    // Set initial stacks

    // Skip CPSR_MODE_USR, which has same sp as SYS mode and would take us out
    // of this privilege level.
    msr CPSR_c, #(CPSR_MODE_FIQ | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =#(KERNEL_STACK_BASE - (0 * KERNEL_STACK_SIZE))
    msr CPSR_c, #(CPSR_MODE_IRQ | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =#(KERNEL_STACK_BASE - (1 * KERNEL_STACK_SIZE))
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =#(KERNEL_STACK_BASE - (2 * KERNEL_STACK_SIZE))
    // Skip CPSR_MODE_MON, as we won't be using secure extensions.
    msr CPSR_c, #(CPSR_MODE_ABT | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =#(KERNEL_STACK_BASE - (3 * KERNEL_STACK_SIZE))
    // Skip CPSR_MODE_HYP, as we won't be using virtualisation extensions.
    msr CPSR_c, #(CPSR_MODE_UND | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =#(KERNEL_STACK_BASE - (4 * KERNEL_STACK_SIZE))
    msr CPSR_c, #(CPSR_MODE_SYS | CPSR_INTERRUPTS_DISABLED)
    ldr sp, =#(KERNEL_STACK_BASE - (5 * KERNEL_STACK_SIZE))

    // Start in supervisor mode
    msr CPSR_c, #(CPSR_MODE_SVC | CPSR_INTERRUPTS_DISABLED)

    // Zero out .bss section
    // We will call  void *memset(void *bytes, int ch, int len)
    ldr r0, =__bss_start__   // void *bytes
    mov r1, #0x0             // int ch
    ldr r12, =__bss_end__
    sub r2, r12, r0          // int len
    bl memset

    /* Copy exception vector to install it */
    mov r0, #0x00                /* r0: copy target */
    ldr r1, =exception_vector     /* r1: copy source */
    ldr r3, =exception_vector_end /* r3: copy end */
keep_copying_exception_vector:
    ldr r2, [r1, #0x0]
    str r2, [r0, #0x0]
    add r0, r0, #0x4
    add r1, r1, #0x4
    cmp r1, r3
    bne keep_copying_exception_vector

    // Set VBAR to #0x0: sets exception vector location to #0x0.
    // E.g. u-boot seems to point this at its own vector.
    mov r0, #0x0
    bl set_vbar

    // Jump to C.
    bl main

exception_vector:
    ldr pc, p_unhandled_exception // reset
    ldr pc, p_unhandled_exception // undefined instruction
    ldr pc, p_svc_entry           // supervisor call
    ldr pc, p_unhandled_exception // prefetch abort
    ldr pc, p_unhandled_exception // data abort
    ldr pc, p_unhandled_exception // hypervisor mode entry
    ldr pc, p_irq_entry           // irq interrupt
    ldr pc, p_unhandled_exception // fiq interrupt
p_svc_entry:           .word svc_entry
p_irq_entry:           .word irq_entry
p_unhandled_exception: .word unhandled_exception
exception_vector_end:

unhandled_exception:
    ldr r0, =unhandled_exception_message
    bl sc_puts
    bl halt

unhandled_exception_message:    .string "\nUnhandled exception! Panic.\n"
