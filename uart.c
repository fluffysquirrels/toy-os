#include "uart.h"

#include "arch_interrupt_numbers.h"
#include "interrupt.h"
#include "kernel.h"
#include "synchronous_console.h"
#include "syscall_handlers.h"
#include "syscalls.h"
#include "uart_registers.h"
#include "util.h"

#ifndef TRACE_UART
#define TRACE_UART 0
#endif

struct uart_t {
  volatile uint32_t *addr;
};

struct uart_t uart_0_ = {
  .addr = (volatile uint32_t *) UART_BASE_0
};
struct uart_t *uart_0 = &uart_0_;

#ifdef UART_BASE_1
struct uart_t uart_1_ = {
  .addr = (volatile uint32_t *) UART_BASE_1
};
struct uart_t *uart_1 = &uart_1_;
#endif // UART_BASE_1

#ifdef UART_BASE_2
struct uart_t uart_2_ = {
  .addr = (volatile uint32_t *) UART_BASE_2
};
struct uart_t *uart_2 = &uart_2_;
#endif // UART_BASE_2

#ifdef UART_BASE_3
struct uart_t uart_3_ = {
  .addr = (volatile uint32_t *) UART_BASE_3
};
struct uart_t *uart_3 = &uart_3_;
#endif // UART_BASE_3

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
    ASSERT(ch >= 0 && ch < 256);
    sc_puts("'");
    sc_putch((char) ch);
    sc_puts("'");
  }
  sc_puts("\n");
}

void uart_log_status(struct uart_t *u) {
  sc_printf("  rx_empty = %s\n",
            uart_rx_fifo_empty(u) ? "true" : "false");

  // Don't read DR or we will empty the fifo.
  //  sc_print_uint32_mem("  UART_DR   ", u->addr + UART_DR   );
  sc_print_uint32_mem("  UART_RSR  ", u->addr + UART_RSR  );
  sc_print_uint32_mem("  UART_ECR  ", u->addr + UART_ECR  );
  sc_print_uint32_mem("  UART_FR   ", u->addr + UART_FR   );
  sc_print_uint32_mem("  UART_ILPR ", u->addr + UART_ILPR );
  sc_print_uint32_mem("  UART_IBRD ", u->addr + UART_IBRD );
  sc_print_uint32_mem("  UART_FBRD ", u->addr + UART_FBRD );
  sc_print_uint32_mem("  UART_LCRH ", u->addr + UART_LCRH );
  sc_print_uint32_mem("  UART_CR   ", u->addr + UART_CR   );
  sc_print_uint32_mem("  UART_IFLS ", u->addr + UART_IFLS );
  sc_print_uint32_mem("  UART_IMSC ", u->addr + UART_IMSC );
  sc_print_uint32_mem("  UART_RIS  ", u->addr + UART_RIS  );
  sc_print_uint32_mem("  UART_MIS  ", u->addr + UART_MIS  );
  sc_print_uint32_mem("  UART_ICR  ", u->addr + UART_ICR  );
  sc_print_uint32_mem("  UART_DMACR", u->addr + UART_DMACR);
}

extern struct file_t uart_0_file;

err_t uart_begin_read(struct uart_t *u) {
  *(u->addr + UART_IMSC) |= UART_RXIM;
  return E_SUCCESS;
}

void uart_0_isr() {
  struct uart_t * u = uart_0;

#if TRACE_UART
  sc_LOG("");
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

static void uart_init_single(struct uart_t *uart) {
  *(uart->addr + UART_CR) |=
    UART_CR_UARTEN |
    UART_CR_RXE |
    UART_CR_TXE;
}

void uart_init() {
  // TODO: Test this interrupt on raspi.
  interrupt_set_handler(INTNUM_UART0, uart_0_isr);
  interrupt_enable(INTNUM_UART0);

#ifdef UART_BASE_0
  uart_init_single(uart_0);
#endif // UART_BASE_0
#ifdef UART_BASE_1
  uart_init_single(uart_1);
#endif // UART_BASE_1
#ifdef UART_BASE_2
  uart_init_single(uart_2);
#endif // UART_BASE_2
#ifdef UART_BASE_3
  uart_init_single(uart_3);
#endif // UART_BASE_3
}
