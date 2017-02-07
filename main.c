#include "context_switch.h"
#include "kernel.h"
#include "heap.h"
#include "interrupt.h"
#include "stdint.h"
#include "stdlib.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "third_party/OpenBSD_collections/src/tree.h"
#include "thread.h"
#include "timer.h"
#include "timer_raspi.h"
#include "timer_sp804.h"
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
void tests_thread();
void sleep_thread();
void heap_stat_thread();

void test_raspi_timers();

int main() {
  sc_puts("main()\n");

  kernel_init();

  struct thread_t *t;

#if CONFIG_ARCH_raspi2
  test_raspi_timers();
#endif

  // ASSERT(kspawn(0x10, &yield_thread, &t) == E_SUCCESS);
  // ASSERT(kspawn(0x10, &spawner_thread, &t) == E_SUCCESS);
  // ASSERT(kspawn(0x10, &return_thread, &t) == E_SUCCESS);
  // ASSERT(kspawn(0x10, &exit_thread, &t) == E_SUCCESS);
  ASSERT(kspawn(0x10, &console_reader_thread, &t) == E_SUCCESS);
  ASSERT(kspawn(0x10, &heap_stat_thread, &t) == E_SUCCESS);
  ASSERT(kspawn(0x10, &sleep_thread, &t) == E_SUCCESS);
  ASSERT(kspawn(0x10, &tests_thread, &t) == E_SUCCESS);

  // ASSERT(kspawn(0x10, &busy_loop_thread, &t) == E_SUCCESS);
  // thread_update_priority(t, 5);
  // ASSERT(kspawn(0x10, &busy_loop_thread, &t) == E_SUCCESS);
  // thread_update_priority(t, 5);

  sc_puts("main() spawned threads\n");

  kernel_run();

  /* Not reached */
  return 0;
}

void test_raspi_timers() {
  sc_LOG("Setting timeouts");
  timer_raspi_set_timeout(1 * DURATION_S);
  timer_sp804_set_timeout(timer_sp804_timer0, 1000000);

  sc_LOG("Delaying");
  timer_delay(3 * DURATION_S);
  sc_LOG("Done delay");

  interrupt_log_status();
  timer_raspi_print_status();
  timer_sp804_log_timer_state(timer_sp804_timer0);

  sc_LOG("sleeping");
  sleep();
  sc_LOG("back from sleep");
}


void tests_thread() {
  ASSERT(sys_invalid() == E_NOSUCHSYSCALL);

  sc_LOG("end");
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
  sc_LOG("");
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
    sc_printf("%x:%x\n", it->k, it->v);
#endif
  }

  struct int_map_node query = { .k = 750 };
  struct int_map_node *result = RB_FIND(int_map, &head, &query);
  ASSERT(result != NULL);
  ASSERT(result->k == 750);
  ASSERT(result->v == 250);
  sc_LOGF("UINT32_MAX=%u", UINT32_MAX);
  sc_LOGF("result @ %x { k=%u v=%u }", result, result->k, result->v);
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

  sc_LOG("end");
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

void busy_loop_thread() {
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

void spawn_one();

void spawner_thread() {
  sc_puts("spawn_thread()\n");

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

void return_thread() {
  sc_puts("return_thread() start\n");
  sys_yield();
  sc_puts("return_thread() end\n");
}

void exit_thread() {
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
    ASSERT(ch >= 0 && ch < 256);
    sc_puts("'");
    sc_putch((char) ch);
    sc_puts("'");
  }
  sc_puts("\n");
}

#ifndef TRACE_CONSOLE_READER
#define TRACE_CONSOLE_READER 0
#endif

void console_reader_thread() {
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
    sc_printf("console_reader_thread() back from read\n"
              "sys_read err = %x\n"
              "sys_read result:\n"
              "  .bytes_read = %x\n"
              "buff = \"%s\"\n"
              , err
              , result.bytes_read,
              buff);
#else  // TRACE_CONSOLE_READER
    sc_printf("console_reader() read '%s'\n", buff);
#endif // TRACE_CONSOLE_READER

    if (buff[0] == 'c') {
      // Clear screen.
      sc_puts("\x1b[2J");
    }
  }
}

void sleep_thread() {
  sc_LOG("start");
  while(1) {
    sc_LOG("loop");
    sys_sleep(DURATION_MS * 100);
  }
}

void heap_stat_thread() {
  sc_LOG("start");
  while(1) {
    sc_print_heap_stats();
    sys_sleep(DURATION_MS * 1000);
  }
}
