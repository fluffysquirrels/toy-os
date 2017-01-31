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

// PIC = Primary Interrupt Controller
/* http://infocenter.arm.com/help/topic/com.arm.doc.dui0224i/I1042232.html */
#define PIC ((volatile unsigned int*)0x10140000)
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
/* http://infocenter.arm.com/help/topic/com.arm.doc.ddi0181e/I1006461.html */
#define VIC_IRQSTATUS    0x0   // 0x0 bytes
#define VIC_FIQSTATUS    0x1   // 0x4 bytes
#define VIC_RAWINTR      0x2   // 0x8 bytes
#define VIC_INTSELECT    0x3   // 0xc bytes
#define VIC_INTENABLE    0x4   // 0x10 bytes
#define VIC_INTENCLEAR   0x5   // 0x14 bytes
#define VIC_SOFTINT      0x6   // 0x18 bytes
#define VIC_SOFTINTCLEAR 0x7   // 0x1c bytes
#define VIC_PROTECTION   0x8   // 0x20 bytes
#define VIC_VECTADDR     0xc   // 0x30 bytes
#define VIC_DEFVECTADDR  0xd   // 0x34 bytes
#define VIC_VECTADDR0    0x40  // 0x100 bytes
#define VIC_VECTADDR1    0x41  // 0x104 bytes
#define VIC_VECTADDR2    0x42  // 0x108 bytes
#define VIC_VECTADDR3    0x43  // 0x10c bytes
#define VIC_VECTADDR4    0x44  // 0x110 bytes
#define VIC_VECTADDR5    0x45  // 0x114 bytes
#define VIC_VECTADDR6    0x46  // 0x118 bytes
#define VIC_VECTADDR7    0x47  // 0x11c bytes
#define VIC_VECTADDR8    0x48  // 0x120 bytes
#define VIC_VECTADDR9    0x49  // 0x124 bytes
#define VIC_VECTADDR10   0x4a  // 0x128 bytes
#define VIC_VECTADDR11   0x4b  // 0x12c bytes
#define VIC_VECTADDR12   0x4c  // 0x130 bytes
#define VIC_VECTADDR13   0x4d  // 0x134 bytes
#define VIC_VECTADDR14   0x4e  // 0x138 bytes
#define VIC_VECTADDR15   0x4f  // 0x13c bytes
#define VIC_VECTCNTL0    0x80  // 0x200 bytes
#define VIC_VECTCNTL1    0x81  // 0x204 bytes
#define VIC_VECTCNTL2    0x82  // 0x208 bytes
#define VIC_VECTCNTL3    0x83  // 0x20c bytes
#define VIC_VECTCNTL4    0x84  // 0x210 bytes
#define VIC_VECTCNTL5    0x85  // 0x214 bytes
#define VIC_VECTCNTL6    0x86  // 0x218 bytes
#define VIC_VECTCNTL7    0x87  // 0x21c bytes
#define VIC_VECTCNTL8    0x88  // 0x220 bytes
#define VIC_VECTCNTL9    0x89  // 0x224 bytes
#define VIC_VECTCNTL10   0x8a  // 0x228 bytes
#define VIC_VECTCNTL11   0x8b  // 0x22c bytes
#define VIC_VECTCNTL12   0x8c  // 0x230 bytes
#define VIC_VECTCNTL13   0x8d  // 0x234 bytes
#define VIC_VECTCNTL14   0x8e  // 0x238 bytes
#define VIC_VECTCNTL15   0x8f  // 0x23c bytes
#define VIC_PERIPHID0    0x3f8 // 0xfe0 bytes
#define VIC_PERIPHID1    0x3f9 // 0xfe4 bytes
#define VIC_PERIPHID2    0x3fa // 0xfe8 bytes
#define VIC_PERIPHID3    0x3fb // 0xfec bytes
#define VIC_PCELLID0     0x3fc // 0xff0 bytes
#define VIC_PCELLID1     0x3fd // 0xff4 bytes
#define VIC_PCELLID2     0x3fe // 0xff8 bytes
#define VIC_PCELLID3     0x3ff // 0xffc bytes
