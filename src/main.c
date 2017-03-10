#include "asm_constants.h"
#include "context_switch.h"
#include "kernel.h"
#include "heap.h"
#include "interrupt.h"
#include "stdint.h"
#include "stdlib.h"
#include "console.h"
#include "syscalls.h"
#include "third_party/OpenBSD_collections/src/tree.h"
#include "thread.h"
#include "timer.h"
#include "timer_sp804.h"
#include "uart.h"
#include "util.h"

void exercise_trees();
void exercise_malloc();
void yield_thread();
void yield_sub(unsigned int);
void busy_loop_thread();
void spawner_thread();
void return_thread();
void exit_thread();
void console_reader_thread();
void blocking_console_reader_thread();
void tests_thread();
void sleep_thread();
void heap_stat_thread();

void run_invalid_opcode();
void print_exception_vector();
void print_sctlr();

void test_printf();

#if CONFIG_ARCH_raspi2
#include "timer_raspi.h"
void test_raspi_timers();
#endif

#if CONFIG_ARCH_stm32f4
void stm32f4_scratch();
#endif

int main() {
  test_printf();

#if CONFIG_ARCH_stm32f4
  stm32f4_scratch();
#endif
  LOG("");

  kernel_init();

#if CONFIG_ARCH_raspi2
  test_raspi_timers();
#endif


  struct thread_t *t;

  // ASSERT(kspawn(CPSR_MODE_USR, &yield_thread, &t) == E_SUCCESS);
  // ASSERT(kspawn(CPSR_MODE_USR, &spawner_thread, &t) == E_SUCCESS);
  // ASSERT(kspawn(CPSR_MODE_USR, &return_thread, &t) == E_SUCCESS);
  // ASSERT(kspawn(CPSR_MODE_USR, &exit_thread, &t) == E_SUCCESS);


  ASSERT(kspawn(CPSR_MODE_USR, &heap_stat_thread, &t) == E_SUCCESS);
//  ASSERT(kspawn(CPSR_MODE_USR, &tests_thread, &t) == E_SUCCESS);
//  ASSERT(kspawn(CPSR_MODE_USR, &return_thread, &t) == E_SUCCESS);
//  ASSERT(kspawn(CPSR_MODE_USR, &exit_thread, &t) == E_SUCCESS);
  ASSERT(kspawn(CPSR_MODE_USR, &sleep_thread, &t) == E_SUCCESS);
//  ASSERT(kspawn(CPSR_MODE_USR, &console_reader_thread, &t) == E_SUCCESS);
//  ASSERT(kspawn(CPSR_MODE_USR, &blocking_console_reader_thread, &t) == E_SUCCESS);

//  ASSERT(kspawn(CPSR_MODE_USR, &busy_loop_thread, &t) == E_SUCCESS);
//  thread_update_priority(t, 5);
  // ASSERT(kspawn(CPSR_MODE_USR, &busy_loop_thread, &t) == E_SUCCESS);
  // thread_update_priority(t, 5);

  puts("main() spawned threads\n");

  kernel_run();

  /* Not reached */
  return 0;
}

void test_printf() {
  LOG("");
  printf("u: %u\n", (unsigned int) 1234567890);
  printf("lu: %lu\n", (long unsigned int) 1234567890);
  printf("llu: %llu\n", (long long unsigned int) 3876543211234567890LL);
  printf("x: %x\n", (unsigned int) 0x12345678);
  printf("lx: %lx\n", (long unsigned int) 0x12345678);
}

void stm32f4_scratch() {
  uint32_t n = 3; // d // a-i (0-8)
  volatile uint32_t *gpion_base = (volatile uint32_t *)(0x4002000 + (0x400 * n)); // 0x4002c00
  volatile uint32_t *gpion_bsrr = gpion_base + (0x18 / 4); // 0x4002c18
  //  gpion_bsrr = 0xffff;
//  led4_pin = 12;
//  led3_pin = 13;
//  led5_pin = 14;
//  led6_pin = 15;

  while (1) {
    for (int n = 0; n < 100000; n++) {}
    *gpion_bsrr = 0xf000;
    for (int n = 0; n < 100000; n++) {}
    *gpion_bsrr = 0xf0000000;
  }
}

void print_sctlr() {
  LOG("");
  uint32_t sctlr = get_sctlr();
  printf("sctlr   = %lx\n", sctlr);
  printf("  VE    = %lx\n", sctlr & (1 << 24));
  printf("  V     = %lx\n", sctlr & (1 << 13));

  LOG("");
  uint32_t vbar = get_vbar();
  printf("  vbar  = %lx\n", vbar);

  sc_print_mem_region((uint32_t *) vbar, 2 * 8 * 4);
}

void print_exception_vector() {
  LOG("");
  sc_print_mem_region((uint32_t *) 0x0, 2 * 8 * 4);
}

void run_invalid_opcode() {
  __asm__ volatile (".word 0xf7f0a000\n");
}

#if CONFIG_ARCH_raspi2

void test_raspi_timers() {
  LOG("Setting raspi timeout");
  timer_raspi_set_timeout(100 * DURATION_MS);

  timer_raspi_print_status();

  LOG("delaying");
  timer_delay(200 * DURATION_MS);
  LOG("back from delay");

  interrupt_log_status();
  timer_raspi_print_status();

  LOG("sleeping");
  sleep();
  LOG("back from sleep");





//   LOG("Setting sp804 timeout");
//   timer_sp804_set_timeout(timer_sp804_timer0, 100 * TIMER_SP804_TICKS_PER_MS);
// 
//   timer_sp804_log_timer_state(timer_sp804_timer0);
// 
//   LOG("delaying");
//   timer_delay(200 * DURATION_MS);
//   LOG("back from delay");
//
//   interrupt_log_status();
//   timer_sp804_log_timer_state(timer_sp804_timer0);
//
//   LOG("sleeping");
//   sleep();
//   LOG("back from sleep");
}
#endif // CONFIG_ARCH_raspi2

void tests_thread() {
  ASSERT(sys_invalid() == E_NOSUCHSYSCALL);

  LOG("end");
}

struct int_map_node {
  RB_ENTRY(int_map_node) rb;
  int k;
  int v;
};

DEFINE_KEY_COMPARER(int_map_cmp, struct int_map_node, int, k)

RB_HEAD(int_map, int_map_node);
RB_GENERATE(int_map, int_map_node, rb, int_map_cmp)

void exercise_trees() {
  LOG("");
  struct int_map head;
  RB_INIT(&head);

  for(int i = 0; i < 1000; i++) {
    struct int_map_node *n = malloc(sizeof(struct int_map_node));
    ASSERT(n != NULL);
    n->k = i;
    n->v = 1000 - i;
    RB_INSERT(int_map, &head, n);
  }
  struct int_map_node *it = NULL;
  RB_FOREACH(it, int_map, &head) {
#if 0
    printf("%x:%x\n", it->k, it->v);
#endif
  }

  struct int_map_node query = { .k = 750 };
  struct int_map_node *result = RB_FIND(int_map, &head, &query);
  ASSERT(result != NULL);
  ASSERT(result->k == 750);
  ASSERT(result->v == 250);
  LOGF("UINT32_MAX=%u", UINT32_MAX);
  LOGF("result @ %x { k=%u v=%u }", result, result->k, result->v);
  // Free tree.
  void *tmp;
  RB_FOREACH_SAFE(it, int_map, &head, tmp) {
    RB_REMOVE(int_map, &head, it);
    free(it);
  }

  // Malloc to check all that stuff was freed.
  // x should be the same as the first node allocated.
  int *x = malloc(sizeof(int));
  ASSERT(x != NULL);
  free(x);
  x = NULL;

  LOG("end");
}

void exercise_malloc() {
#define NUM_ALLOCS 20
  size_t size = 500;
  char *allocs[NUM_ALLOCS] = { 0 };

  for (int i = 0; i < NUM_ALLOCS; i++) {
    allocs[i] = (char *) malloc(size);
  }

  for (int i = 0; i < NUM_ALLOCS; i++) {
    free(allocs[i]);
    allocs[i] = NULL;
  }

  for (int i = 0; i < NUM_ALLOCS; i++) {
    allocs[i] = (char *) malloc(size);
  }

  for (int i = 0; i < NUM_ALLOCS; i++) {
    free(allocs[i]);
    allocs[i] = NULL;
  }

#undef NUM_ALLOCS
}

void yield_thread() {
  puts("Start yield()\n");
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
    puts("In yield() loop\n");
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
  puts("In yield_sub() 1\n");
  sys_yield();
  puts("In yield_sub() 2\n");
 }

void busy_loop_thread() {
  LOG("Start");
//    __asm__ volatile(
//    "mov    r0,  #10 " "\n\t"
//    "mov    r1,  #1  " "\n\t"
//    "mov    r2,  #2  " "\n\t"
//    "mov    r3,  #3  " "\n\t"
//    "mov    r4,  #4  " "\n\t"
//    "mov    r5,  #5  " "\n\t"
//    "mov    r6,  #6  " "\n\t"
//    "mov    r7,  #7  " "\n\t"
//    "mov    r8,  #8  " "\n\t"
//    "mov    r9,  #9  " "\n\t"
//    "mov    r10, #10 " "\n\t"
//    "mov    r12, #12 " "\n\t"
//    :
//    :
//    : "cc",
//      "r0", "r1", "r2" , "r3" ,
//      "r4", "r5", "r6" , "r7" ,
//      "r8", "r9", "r10", "r12"
//  );
  while(1) {
    timer_delay(400 * DURATION_MS);
    LOG("tick");
  }
}

void spawn_one();

void spawner_thread() {
  puts("spawn_thread()\n");

  for (int i = 0; i < 3; i++) {
    spawn_one();
  }
  //  test_fail_case();
  while(1) {
    sys_yield();
  }
}

void spawn_one() {
  struct spawn_args_t args = {
    .pc = &yield_thread,
  };
  struct spawn_result_t result;
  err_t ret = sys_spawn(&args, &result);

  puts("spawn_thread() sys_spawn returned\n");
  puts("  result.thread_id = ");
  sc_print_uint32_hex(result.thread_id);
  puts("  ret = ");
  sc_print_uint32_hex(ret);
  puts("\n");

  if(ret == E_SUCCESS) {
    puts("spawn_thread() sys_spawn success\n");
  } else {
    puts("spawn_thread() sys_spawn error\n");
  }
}

void return_thread() {
  puts("return_thread() start\n");
  sys_yield();
  puts("return_thread() end\n");
}

void exit_thread() {
  puts("exit_thread() start\n");
  sys_exit();
}

void log_ch(int ch) {
  puts("  getch got ");
  sc_print_uint32_hex(ch);
  puts(" = ");
  if (ch == -1) {
    puts("EOF");
  } else {
    ASSERT(ch >= 0 && ch < 256);
    puts("'");
    sc_putch((char) ch);
    puts("'");
  }
  puts("\n");
}

#ifndef TRACE_CONSOLE_READER
#define TRACE_CONSOLE_READER 0
#endif

void console_reader_thread() {
  puts("console_reader_thread()\n");

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
    puts("\nconsole_reader_thread() about to read\n");
    puts("sys_read args:\n");
    puts("  .fd = ");
    sc_print_uint32_hex(args.fd);
    puts("\n");
    puts("  .buff = ");
    sc_print_uint32_hex((unsigned int) args.buff);
    puts("\n");
    puts("  .len = ");
    sc_print_uint32_hex(args.len);
    puts("\n");
#endif // TRACE_CONSOLE_READER

    err_t err = sys_read(&args, &result);
    if (err != E_SUCCESS) {
      LOGF("sys_read err = %u", err);
      break;
    }

#if TRACE_CONSOLE_READER
    printf("console_reader_thread() back from read\n"
              "sys_read err = %x\n"
              "sys_read result:\n"
              "  .bytes_read = %x\n"
              "buff = \"%s\"\n"
              , err
              , result.bytes_read,
              buff);
#else  // TRACE_CONSOLE_READER
    printf("console_reader() read '%s'\n", buff);
#endif // TRACE_CONSOLE_READER

    if (buff[0] == 'c') {
      // Clear screen.
      puts("\x1b[2J");
    }
  }
}

void blocking_console_reader_thread() {
  LOG("");
  struct uart_t *u = uart_0;
  while (1) {
    if (!uart_rx_fifo_empty(u)) {
      iochar_t ch = uart_getch(u);
      ASSERT(ch != EOF);
      LOGF("read '%c'", (char)ch);
    }
  }
}

void sleep_thread() {
  LOG("start");
  while(1) {
    LOG("loop");
    sys_sleep(DURATION_MS * 500);
  }
}

void heap_stat_thread() {
  LOG("start");
  while(1) {
    sc_print_heap_stats();
    sys_sleep(DURATION_MS * 1000);
  }
}
