#include "stdlib.h"
#include "synchronous_console.h"

// See also config and shims in third_party/FreeRTOS/FreeRTOS.h

#ifndef TRACE_MALLOC
#define TRACE_MALLOC 0
#endif

extern void *pvPortMalloc(size_t s);
extern void vPortFree(void *p);

void* malloc(size_t size) {
  void *rv = pvPortMalloc(size);
  sc_LOGF_IF(TRACE_MALLOC, "size=%x rv=%x", size, rv);
  return rv;
}

void free(void *p) {
  sc_LOGF_IF(TRACE_MALLOC, "p=%x", p);
  vPortFree(p);
}
