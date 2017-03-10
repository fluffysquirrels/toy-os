/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html */
#define TIMER_LOAD    0x0 /* 0x00 bytes */
#define TIMER_VALUE   0x1 /* 0x04 bytes */
#define TIMER_CONTROL 0x2 /* 0x08 bytes */
#define TIMER_INTCLR  0x3 /* 0x0C bytes */
#define TIMER_RIS     0x4 /* 0x10 bytes */
#define TIMER_MIS     0x5 /* 0x14 bytes */
#define TIMER_BGLOAD  0x6 /* 0x18 bytes */
/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/Babgabfg.html */
#define TIMER_CONTROL_EN       0x80
#define TIMER_CONTROL_PERIODIC 0x40
#define TIMER_CONTROL_INTEN    0x20
#define TIMER_CONTROL_32BIT    0x02
#define TIMER_CONTROL_ONESHOT  0x01
