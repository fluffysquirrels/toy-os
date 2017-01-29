#include "uart.h"
#include "versatilepb.h"

struct uart_t {
  unsigned int *addr;
};

struct uart_t uart_0_ = {
  .addr = (unsigned int *) UART0
};

struct uart_t *uart_0 = &uart_0_;

void uart_sync_putch(struct uart_t *uart, char ch) {
  while(*(uart->addr + UARTFR) & UARTFR_TXFF);
  *UART0 = ch;
}
