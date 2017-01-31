#pragma once

#define UART_BASE_0          ((volatile unsigned int*) 0x101f1000)
#define UART_BASE_1          ((volatile unsigned int*) 0x101f2000)
#define UART_BASE_2          ((volatile unsigned int*) 0x101f3000)
#define UART_BASE_3          ((volatile unsigned int*) 0x10009000)

#define UART_DR              (0x00 / sizeof(unsigned int))   /* Data read or written from the interface. */
#define UART_RSR             (0x04 / sizeof(unsigned int))   /* Receive status register (Read). */
#define UART_ECR             (0x04 / sizeof(unsigned int))   /* Error clear register (Write). */
#define UART_FR              (0x18 / sizeof(unsigned int))   /* Flag register (Read only). */
#define UART_ILPR            (0x20 / sizeof(unsigned int))   /* IrDA low power counter register. */
#define UART_IBRD            (0x24 / sizeof(unsigned int))   /* Integer baud rate divisor register. */
#define UART_FBRD            (0x28 / sizeof(unsigned int))   /* Fractional baud rate divisor register. */
#define UART_LCRH            (0x2c / sizeof(unsigned int))   /* Line control register. */
#define UART_CR              (0x30 / sizeof(unsigned int))   /* Control register. */
#define UART_IFLS            (0x34 / sizeof(unsigned int))   /* Interrupt fifo level select. */
#define UART_IMSC            (0x38 / sizeof(unsigned int))   /* Interrupt mask. */
#define UART_RIS             (0x3c / sizeof(unsigned int))   /* Raw interrupt status. */
#define UART_MIS             (0x40 / sizeof(unsigned int))   /* Masked interrupt status. */
#define UART_ICR             (0x44 / sizeof(unsigned int))   /* Interrupt clear register. */
#define UART_DMACR           (0x48 / sizeof(unsigned int))   /* DMA control register. */

#define UART_DR_OE           (1 << 11)
#define UART_DR_BE           (1 << 10)
#define UART_DR_PE           (1 << 9)
#define UART_DR_FE           (1 << 8)

#define UART_RSR_OE          0x08
#define UART_RSR_BE          0x04
#define UART_RSR_PE          0x02
#define UART_RSR_FE          0x01

#define UART_FR_RI           0x100
#define UART_FR_TXFE         0x080
#define UART_FR_RXFF         0x040
#define UART_FR_TXFF         0x020
#define UART_FR_RXFE         0x010
#define UART_FR_BUSY         0x008
#define UART_FR_DCD          0x004
#define UART_FR_DSR          0x002
#define UART_FR_CTS          0x001

#define UART_CR_CTSEN        0x8000  /* CTS hardware flow control */
#define UART_CR_RTSEN        0x4000  /* RTS hardware flow control */
#define UART_CR_OUT2         0x2000  /* OUT2 */
#define UART_CR_OUT1         0x1000  /* OUT1 */
#define UART_CR_RTS          0x0800  /* RTS */
#define UART_CR_DTR          0x0400  /* DTR */
#define UART_CR_RXE          0x0200  /* receive enable */
#define UART_CR_TXE          0x0100  /* transmit enable */
#define UART_CR_LBE          0x0080  /* loopback enable */
#define UART_CR_IIRLP        0x0004  /* SIR low power mode */
#define UART_CR_SIREN        0x0002  /* SIR enable */
#define UART_CR_UARTEN       0x0001  /* UART enable */

#define UART_LCRH_SPS        0x80
#define UART_LCRH_WLEN_8     0x60
#define UART_LCRH_WLEN_7     0x40
#define UART_LCRH_WLEN_6     0x20
#define UART_LCRH_WLEN_5     0x00
#define UART_LCRH_FEN        0x10
#define UART_LCRH_STP2       0x08
#define UART_LCRH_EPS        0x04
#define UART_LCRH_PEN        0x02
#define UART_LCRH_BRK        0x01

#define UART_IFLS_RX1_8      (0 << 3)
#define UART_IFLS_RX2_8      (1 << 3)
#define UART_IFLS_RX4_8      (2 << 3)
#define UART_IFLS_RX6_8      (3 << 3)
#define UART_IFLS_RX7_8      (4 << 3)
#define UART_IFLS_TX1_8      (0 << 0)
#define UART_IFLS_TX2_8      (1 << 0)
#define UART_IFLS_TX4_8      (2 << 0)
#define UART_IFLS_TX6_8      (3 << 0)
#define UART_IFLS_TX7_8      (4 << 0)

#define UART_OEIM            (1 << 10)       /* overrun error interrupt mask */
#define UART_BEIM            (1 << 9)        /* break error interrupt mask */
#define UART_PEIM            (1 << 8)        /* parity error interrupt mask */
#define UART_FEIM            (1 << 7)        /* framing error interrupt mask */
#define UART_RTIM            (1 << 6)        /* receive timeout interrupt mask */
#define UART_TXIM            (1 << 5)        /* transmit interrupt mask */
#define UART_RXIM            (1 << 4)        /* receive interrupt mask */
#define UART_DSRMIM          (1 << 3)        /* DSR interrupt mask */
#define UART_DCDMIM          (1 << 2)        /* DCD interrupt mask */
#define UART_CTSMIM          (1 << 1)        /* CTS interrupt mask */
#define UART_RIMIM           (1 << 0)        /* RI interrupt mask */

#define UART_OEIS            (1 << 10)       /* overrun error interrupt status */
#define UART_BEIS            (1 << 9)        /* break error interrupt status */
#define UART_PEIS            (1 << 8)        /* parity error interrupt status */
#define UART_FEIS            (1 << 7)        /* framing error interrupt status */
#define UART_RTIS            (1 << 6)        /* receive timeout interrupt status */
#define UART_TXIS            (1 << 5)        /* transmit interrupt status */
#define UART_RXIS            (1 << 4)        /* receive interrupt status */
#define UART_DSRMIS          (1 << 3)        /* DSR interrupt status */
#define UART_DCDMIS          (1 << 2)        /* DCD interrupt status */
#define UART_CTSMIS          (1 << 1)        /* CTS interrupt status */
#define UART_RIMIS           (1 << 0)        /* RI interrupt status */

#define UART_OEIC            (1 << 10)       /* overrun error interrupt clear */
#define UART_BEIC            (1 << 9)        /* break error interrupt clear */
#define UART_PEIC            (1 << 8)        /* parity error interrupt clear */
#define UART_FEIC            (1 << 7)        /* framing error interrupt clear */
#define UART_RTIC            (1 << 6)        /* receive timeout interrupt clear */
#define UART_TXIC            (1 << 5)        /* transmit interrupt clear */
#define UART_RXIC            (1 << 4)        /* receive interrupt clear */
#define UART_DSRMIC          (1 << 3)        /* DSR interrupt clear */
#define UART_DCDMIC          (1 << 2)        /* DCD interrupt clear */
#define UART_CTSMIC          (1 << 1)        /* CTS interrupt clear */
#define UART_RIMIC           (1 << 0)        /* RI interrupt clear */

#define UART_DMAONERR        (1 << 2)        /* disable dma on error */
#define UART_TXDMAE          (1 << 1)        /* enable transmit dma */
#define UART_RXDMAE          (1 << 0)        /* enable receive dma */
