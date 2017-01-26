#define UART0 ((volatile unsigned int*) 0x101f1000)
#define UARTFR (0x18 / sizeof(unsigned int))
#define UARTFR_TXFF 0x20
