#include "kernel.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "uart.h"
#include "uart_consts.h"
#include "util.h"
#include "versatilepb.h"

#define TRACE_UART 0

struct uart_t {
  unsigned int *addr;
};

struct uart_t uart_0_ = {
  .addr = (unsigned int *) UART_BASE_0
};
struct uart_t *uart_0 = &uart_0_;

struct uart_t uart_1_ = {
  .addr = (unsigned int *) UART_BASE_1
};
struct uart_t *uart_1 = &uart_1_;

struct uart_t uart_2_ = {
  .addr = (unsigned int *) UART_BASE_2
};
struct uart_t *uart_2 = &uart_2_;

struct uart_t uart_3_ = {
  .addr = (unsigned int *) UART_BASE_3
};
struct uart_t *uart_3 = &uart_3_;

bool uart_tx_fifo_full(struct uart_t *uart) {
  return (*(uart->addr + UART_FR) & UART_FR_TXFF);
}

bool uart_rx_fifo_empty(struct uart_t *uart) {
  return (*(uart->addr + UART_FR) & UART_FR_RXFE);
}

void uart_sync_putch(struct uart_t *uart, char ch) {
  while(uart_tx_fifo_full(uart)) {}
  *(uart->addr + UART_DR) = ch;
}

iochar_t uart_getch(struct uart_t *uart) {
  if (uart_rx_fifo_empty(uart)) {
    return -1;
  }

  return *(uart->addr + UART_DR);
}

void uart_log_getch(iochar_t ch) {
  sc_puts("  getch got ");
  sc_print_uint32_hex(ch);
  sc_puts(" = ");
  if (ch == -1) {
    sc_puts("EOF");
  } else {
    assert(ch >= 0 && ch < 256, "ch >= 0 && ch < 256");
    sc_puts("'");
    sc_putch((char) ch);
    sc_puts("'");
  }
  sc_puts("\n");
}

void uart_log_status(struct uart_t *u) {
  sc_puts("  rx_empty = ");
  sc_puts(uart_rx_fifo_empty(u) ? "true" : "false");
  sc_puts("\n");

  sc_puts("  FR = ");
  sc_print_uint32_hex(*(u->addr + UART_FR));
  sc_puts("\n");

  sc_puts("  CR = ");
  sc_print_uint32_hex(*(u->addr + UART_CR));
  sc_puts("\n");

  sc_puts("  RIS = ");
  sc_print_uint32_hex(*(u->addr + UART_RIS));
  sc_puts("\n");

  sc_puts("  MIS = ");
  sc_print_uint32_hex(*(u->addr + UART_MIS));
  sc_puts("\n");

  sc_puts("  IMSC = ");
  sc_print_uint32_hex(*(u->addr + UART_IMSC));
  sc_puts("\n");
}

extern struct file_t uart_0_file;

err_t uart_begin_read(struct uart_t *u) {
  *(u->addr + UART_IMSC) |= UART_RXIM;
  return E_SUCCESS;
}

void uart_0_isr() {
  struct uart_t * u = uart_0;

#if TRACE_UART
  sc_puts("\nuart_0_isr()\n");
  uart_log_status(u);
#endif // TRACE_UART

  // Disable interrupt on uart_0 rx
  // This is part of cancelling future interrupts and reads
  // Disable it before sysh_read_callback might run as that callback
  // may decide to read some more.
  *(u->addr + UART_IMSC) &= ~UART_RXIM;

  if(uart_0_file.read_callback_registered) {
    // Disable callback.
    // Disable it before sysh_read_callback runs as that callback
    // may decide to read some more.
    uart_0_file.read_callback_registered = false;
    sysh_read_callback(&(uart_0_file.read_callback_state));
  }

#if TRACE_UART
  uart_log_status(u);
#endif // TRACE_UART
}

void uart_init() {
  set_interrupt_handler(PIC_INTNUM_UART0, uart_0_isr);

  // Enable interrupt on controller
  *(PIC + VIC_INTENABLE) |= PIC_INTMASK_UART0;
}
