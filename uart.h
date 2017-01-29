struct uart_t;
extern struct uart_t *uart_0;

void uart_sync_putch(struct uart_t *, char);
