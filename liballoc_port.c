#include "synchronous_console.h"
#include "util.h"

// liballoc_port implements the functions liballoc needs
// See third_party/liballoc/liballoc.h

int liballoc_lock(){
  // Interrupts are already disabled in the kernel. No-op.
  return 0;
}
int liballoc_unlock(){
  // Interrupts are already disabled in the kernel. No-op.
  return 0;
}

static char *heap_min = (char *) 0x07ffffff;

#define PAGE_SIZE 4096

void* liballoc_alloc( int pages ) {
  sc_LOGF("pages = %x heap_min = %x", pages, heap_min);
  heap_min -= pages * PAGE_SIZE;
  return heap_min;
}
int liballoc_free( void* ptr, int pages ) {
  sc_LOGF("ptr = %x pages = %x", ptr, pages);


  // TODO: Implement this.
  UNUSED(ptr);
  UNUSED(pages);

  return -1;
}

int raise(int sig) {
  UNUSED(sig);
  PANIC("Not implemented");

  // Not reached.
  return 0;
}
