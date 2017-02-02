#include "kernel.h"
#include "stdint.h"
#include "stdlib.h"
#include "synchronous_console.h"
#include "syscalls.h"
#include "third_party/OpenBSD_collections/src/tree.h"
#include "thread.h"
#include "util.h"

void exercise_trees(void);
void exercise_malloc(void);
void yield_thread(void);
void yield_sub(unsigned int);
void busy_loop_thread(void);
void spawner_thread(void);
void return_thread(void);
void exit_thread(void);
void console_reader_thread(void);

int main(void) {
  sc_puts("main()\n");

  exercise_malloc();
  exercise_trees();

  kernel_init();

  //  kspawn(0x10, &yield_thread, NULL);
  /* kspawn(0x10, &busy_loop_thread, NULL); */
  /* //  kspawn(0x10, &spawner_thread, NULL); */
  /* kspawn(0x10, &return_thread, NULL); */
  /* kspawn(0x10, &exit_thread, NULL); */
  struct thread_t *crt;
  kspawn(0x10, &console_reader_thread, &crt);
  thread_update_priority(crt, 20);
  kspawn(0x10, &busy_loop_thread, NULL);

  sc_puts("main() spawned threads\n");

  kernel_run();

  /* Not reached */
  return 0;
}

struct int_map_node {
  RB_ENTRY(int_map_node) rb;
  int k;
  int v;
};

#define DEF_KEY_CMP(func_name, elt_t, key_t, key_member)\
int func_name(elt_t *e1, elt_t *e2) {\
  key_t k1 = e1->key_member;\
  key_t k2 = e2->key_member;\
  return k1 < k2 ? -1\
       : k1 > k2 ? +1\
       : 0;\
}

DEF_KEY_CMP(int_map_cmp2, struct int_map_node, int, k)

RB_HEAD(int_map, int_map_node);
RB_GENERATE(int_map, int_map_node, rb, int_map_cmp2)

void exercise_trees(void) {
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

void exercise_malloc(void) {
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
