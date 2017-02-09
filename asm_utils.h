#pragma once

#if TRACE_ASM
#define logch(ch)     \
  push {r0-r3,ip,lr}; \
    ldr r0, =ch;      \
    bl sc_putch;      \
    pop {r0-r3,ip,lr};
#else
#define logch(ch)
#endif
