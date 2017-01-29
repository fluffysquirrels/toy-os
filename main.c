#include "kernel.h"
#include "synchronous_console.h"
#include "syscalls.h"

void first(void);
void first_sub(unsigned int);
void second(void);
void spawner(void);

int main(void) {
  kspawn(0x10, &first, NULL);
  kspawn(0x10, &second, NULL);
  kspawn(0x10, &spawner, NULL);

  sc_puts("Hello, World from main!\n");

  scheduler_run();

  /* Not reached */
  return 0;
}

/* First user mode program */
void first(void) {
  sc_puts("Start first()\n");
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
    sc_puts("In first() loop\n");
    n++;
    first_sub(n);

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

void first_sub(unsigned int arg1) {
  UNUSED(arg1);
  sc_puts("In first_sub() 1\n");
  sys_yield();
  sc_puts("In first_sub() 2\n");
 }

/* Second user mode program */
void second(void) {
  sc_puts("Start second()\n");
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
    "mov    r11, #11 " "\n\t"
    "mov    r12, #12 " "\n\t"
    :
    :
    : "cc",
      "r0", "r1", "r2" , "r3" ,
      "r4", "r5", "r6" , "r7" ,
      "r8", "r9", "r10", "r12"
  );
  while(1) {
  }
}

void spawn_one(void);

void spawner(void) {
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
    .pc = &first,
  };
  struct spawn_result_t result;
  syscall_error_t ret = sys_spawn(&args, &result);

  sc_puts("spawn_thread() sys_spawn returned\n");
  sc_puts("  result.thread_id = ");
  sc_print_uint32_hex(result.thread_id);
  sc_puts("  ret = ");
  sc_print_uint32_hex(ret);
  sc_puts("\n");

  if(ret == SE_SUCCESS) {
    sc_puts("spawn_thread() sys_spawn success\n");
  } else {
    sc_puts("spawn_thread() sys_spawn error\n");
  }
}
