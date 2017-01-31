#pragma once
#include "syscalls.h"

struct uart_t;
extern struct uart_t *uart_0;

void uart_sync_putch(struct uart_t *, char);
void uart_init();
iochar_t uart_getch(struct uart_t *);
err_t uart_begin_read(struct uart_t *u);
