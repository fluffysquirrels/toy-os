#pragma once

/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html */
#define TIMER0 ((volatile unsigned int*)0x101E2000)
#define TIMER1 ((volatile unsigned int*)0x101E2020)
#define TIMER2 ((volatile unsigned int*)0x101E3000)
#define TIMER3 ((volatile unsigned int*)0x101E3020)
#define TIMER_LOAD 0x0 /* 0x00 bytes */
#define TIMER_VALUE 0x1 /* 0x04 bytes */
#define TIMER_CONTROL 0x2 /* 0x08 bytes */
#define TIMER_INTCLR 0x3 /* 0x0C bytes */
#define TIMER_MIS 0x5 /* 0x14 bytes */
#define TIMER_BGLOAD 0x6 /* 0x18 bytes */
/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/Babgabfg.html */
#define TIMER_EN 0x80
#define TIMER_PERIODIC 0x40
#define TIMER_INTEN 0x20
#define TIMER_32BIT 0x02
#define TIMER_ONESHOT 0x01

// Interrupt numbers
/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/Chdbeibh.html */
#define PIC_INTNUM_COUNT    32
#define PIC_INTNUM_TIMER01  4 // Interrupt for timers 0 and 1
#define PIC_INTMASK_TIMER01 (1 << 4)
#define PIC_INTNUM_TIMER23  5 // Interrupt for timers 2 and 3
#define PIC_INTNUM_RTC      10
#define PIC_INTNUM_UART0    12
#define PIC_INTMASK_UART0   (1 << 12)
#define PIC_INTNUM_UART1    13
#define PIC_INTNUM_UART2    14
#define PIC_INTNUM_DMA      17
