#pragma once

#if CONFIG_ARCH_stm32f4
#define KERNEL_STACK_BASE (0x20000000 + 192 * 1024 - 0x04) // End of RAM
#else
#define KERNEL_STACK_BASE 0x07fffff0
#endif

#define KERNEL_STACK_SIZE 4 * 1024 // 4 KB

#define ACTIVATE_RET_IRQ 0x1
#define ACTIVATE_RET_SYSCALL 0x2

#define SYSCALL_NUM_YIELD 0x1
#define SYSCALL_NUM_SPAWN 0x2
#define SYSCALL_NUM_EXIT  0x3
#define SYSCALL_NUM_READ  0x4
#define SYSCALL_NUM_SLEEP 0x5
#define SYSCALL_NUM_MAX   0x5 // Should be equal to the highest syscall number
                              // Used to allocate syscall handler table

#define THREAD_OFFSET_cpsr 0x0
#define THREAD_OFFSET_registers 0x4

// See page 1139 of ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition version C.c
#define CPSR_MODE_USR 0x10 // User mode
#define CPSR_MODE_FIQ 0x11 // FIQ mode
#define CPSR_MODE_IRQ 0x12 // IRQ mode
#define CPSR_MODE_SVC 0x13 // Supervisor mode
#define CPSR_MODE_MON 0x16 // Monitor mode
#define CPSR_MODE_ABT 0x17 // Abort mode
#define CPSR_MODE_HYP 0x1a // Hypervisor mode
#define CPSR_MODE_UND 0x1b // Undefined mode
#define CPSR_MODE_SYS 0x1f // System mode

#define CPSR_IRQ_DISABLED (1 << 7)
#define CPSR_FIQ_DISABLED (1 << 6)
#define CPSR_INTERRUPTS_DISABLED (CPSR_IRQ_DISABLED | CPSR_FIQ_DISABLED)
