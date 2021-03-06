#include "heap.h"

#include "third_party/FreeRTOS/port.h"
#include "stdlib.h"
#include "console.h"

// See also config and shims in third_party/FreeRTOS/FreeRTOS.h

#ifndef TRACE_MALLOC
#define TRACE_MALLOC 0
#endif

void* malloc(size_t size) {
  void *rv = pvPortMalloc(size);
  LOGF_IF(TRACE_MALLOC, "size=%x rv=%x", size, rv);
  return rv;
}

void free(void *p) {
  LOGF_IF(TRACE_MALLOC, "p=%x", p);
  vPortFree(p);
}

size_t heap_get_free() {
  return xPortGetFreeHeapSize();
}

size_t heap_get_min_free() {
  return xPortGetMinimumEverFreeHeapSize();
}

void sc_print_heap_stats() {
  printf(
    "Heap stats:\n"
    "  Free:         %u\n"
    "  Minimum free: %u\n",
    heap_get_free(),
    heap_get_min_free());
}
