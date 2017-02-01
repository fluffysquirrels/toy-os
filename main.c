#include "kernel.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "thread.h"
#include "util.h"

void yield_thread(void);
void yield_sub(unsigned int);
void busy_loop_thread(void);
void spawner_thread(void);
void return_thread(void);
void exit_thread(void);
void console_reader_thread(void);

int main(void) {
  sc_puts("main()\n");

  kernel_init();

  //  kspawn(0x10, &yield_thread, NULL);
  kspawn(0x10, &busy_loop_thread, NULL);
//  kspawn(0x10, &spawner_thread, NULL);
//  kspawn(0x10, &return_thread, NULL);
//  kspawn(0x10, &exit_thread, NULL);
  struct thread_t *crt;
  kspawn(0x10, &console_reader_thread, &crt);
  thread_update_priority(crt, 20);
  kspawn(0x10, &busy_loop_thread, NULL);

  sc_puts("main() spawned threads\n");

  kernel_run();

  /* Not reached */
  return 0;
}

void yield_thread(void) {
  sc_puts("Start yield()\n");
  __asm__ volatile(
    "mov    r0,  #10 " "\n\t"
    "mov    r1,  #1  " "\n\t"
    "mov    r2,  #2  " "\n\t"
    "mov    r3,  #3  " "\n\t"
    "mov    r4,  #4  " "\n\t"
    "mov    r5,  #5  " "\n\t"
    "mov    r6,  #6  " "\n\t"
    "mov    r7,  #7  " "\n\t"
    "mov    r8,  #8  " "\n\t"
    "mov    r9,  #9  " "\n\t"
    "mov    r10, #10 " "\n\t"
    "mov    r12, #12 " "\n\t"
    :
    :
    : "cc",
      "r0", "r1", "r2" , "r3" ,
      "r4", "r5", "r6" , "r7" ,
      "r8", "r9", "r10", "r12"
  );
  sys_yield();
  unsigned int n = 17;
  while(1) {
    sc_puts("In yield() loop\n");
    n++;
    yield_sub(n);

    __asm__ volatile(
      "mov    r0,  #10 " "\n\t"
      "mov    r1,  #1  " "\n\t"
      "mov    r2,  #2  " "\n\t"
      "mov    r3,  #3  " "\n\t"
      "mov    r4,  #4  " "\n\t"
      "mov    r5,  #5  " "\n\t"
      "mov    r6,  #6  " "\n\t"
      "mov    r7,  #7  " "\n\t"
      "mov    r8,  #8  " "\n\t"
      "mov    r9,  #9  " "\n\t"
      "mov    r10, #10 " "\n\t"
      "mov    r12, #12 " "\n\t"
      :
      :
      : "cc",
        "r0", "r1", "r2" , "r3" ,
        "r4", "r5", "r6" , "r7" ,
        "r8", "r9", "r10", "r12"
    );
    sys_yield();
  }
}

void yield_sub(unsigned int arg1) {
  UNUSED(arg1);
  sc_puts("In yield_sub() 1\n");
  sys_yield();
  sc_puts("In yield_sub() 2\n");
 }

void busy_loop_thread(void) {
  sc_puts("Start busy_loop_thread()\n");
    __asm__ volatile(
    "mov    r0,  #10 " "\n\t"
    "mov    r1,  #1  " "\n\t"
    "mov    r2,  #2  " "\n\t"
    "mov    r3,  #3  " "\n\t"
    "mov    r4,  #4  " "\n\t"
    "mov    r5,  #5  " "\n\t"
    "mov    r6,  #6  " "\n\t"
    "mov    r7,  #7  " "\n\t"
    "mov    r8,  #8  " "\n\t"
    "mov    r9,  #9  " "\n\t"
    "mov    r10, #10 " "\n\t"
    "mov    r12, #12 " "\n\t"
    :
    :
    : "cc",
      "r0", "r1", "r2" , "r3" ,
      "r4", "r5", "r6" , "r7" ,
      "r8", "r9", "r10", "r12"
  );
  while(1) {
    int n = 0;

    for(n = 0; n < 100000000; n = n + 1) {}
    sc_puts("busy_loop() tick\n");
  }
}

void spawn_one(void);

void spawner_thread(void) {
  sc_puts("spawn_thread()\n");

  for (int i = 0; i < 3; i++) {
    spawn_one();
  }
  //  test_fail_case();
  while(1) {
    sys_yield();
  }
}

void spawn_one(void) {
  struct spawn_args_t args = {
    .pc = &yield_thread,
  };
  struct spawn_result_t result;
  err_t ret = sys_spawn(&args, &result);

  sc_puts("spawn_thread() sys_spawn returned\n");
  sc_puts("  result.thread_id = ");
  sc_print_uint32_hex(result.thread_id);
  sc_puts("  ret = ");
  sc_print_uint32_hex(ret);
  sc_puts("\n");

  if(ret == E_SUCCESS) {
    sc_puts("spawn_thread() sys_spawn success\n");
  } else {
    sc_puts("spawn_thread() sys_spawn error\n");
  }
}

void return_thread(void) {
  sc_puts("return_thread() start\n");
  sys_yield();
  sc_puts("return_thread() end\n");
}

void exit_thread(void) {
  sc_puts("exit_thread() start\n");
  sys_exit();
}

void log_ch(int ch) {
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

#ifndef TRACE_CONSOLE_READER
#define TRACE_CONSOLE_READER 0
#endif

void console_reader_thread(void) {
  sc_puts("console_reader_thread()\n");

  char buff[16];
  struct read_args_t args = {
    .fd = -1, // TODO
    .buff = buff,
    .len = sizeof(buff) - 1, // Leave final byte of buff as null terminator
  };
  struct read_result_t result;

  while(1) {
    memset(buff, '\0', sizeof(buff));
    result.bytes_read = -1;
#if TRACE_CONSOLE_READER
    sc_puts("\nconsole_reader_thread() about to read\n");
    sc_puts("sys_read args:\n");
    sc_puts("  .fd = ");
    sc_print_uint32_hex(args.fd);
    sc_puts("\n");
    sc_puts("  .buff = ");
    sc_print_uint32_hex((unsigned int) args.buff);
    sc_puts("\n");
    sc_puts("  .len = ");
    sc_print_uint32_hex(args.len);
    sc_puts("\n");
#endif // TRACE_CONSOLE_READER

    err_t err = sys_read(&args, &result);
    UNUSED(err);

#if TRACE_CONSOLE_READER
    sc_puts("console_reader_thread() back from read\n");
    sc_puts("sys_read err = ");
    sc_print_uint32_hex(err);
    sc_puts("\n");
    sc_puts("sys_read result:\n");
    sc_puts("  .bytes_read = ");
    sc_print_uint32_hex(result.bytes_read);
    sc_puts("\n");

    log_ch(buff[0]);
#else  // TRACE_CONSOLE_READER
    sc_puts("console_reader() read '");
    sc_puts(buff);
    sc_puts("'\n");
#endif // TRACE_CONSOLE_READER

    if (buff[0] == 'c') {
      // Clear screen.
      sc_puts("\x1b[2J");
    }
  }
}
