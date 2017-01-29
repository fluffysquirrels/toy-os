#define ACTIVATE_RET_IRQ 0x1
#define ACTIVATE_RET_SYSCALL 0x2

#define SYSCALL_NUM_YIELD 0x1
#define SYSCALL_NUM_SPAWN 0x2
#define SYSCALL_NUM_EXIT  0x3
#define SYSCALL_NUM_MAX   0x3 // Should be equal to the highest syscall number
                              // Used to allocate syscall handler table

#define THREAD_OFFSET_cpsr 0x0
#define THREAD_OFFSET_registers 0x4
