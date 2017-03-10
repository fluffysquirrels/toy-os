// The minimal definitions required to get the FreeRTOS heaps working.

#include "stdint.h"
#include "util.h"
#include "console.h"

#define configTOTAL_HEAP_SIZE 10 * 1024 * 1024

#define portBYTE_ALIGNMENT 8
#define portBYTE_ALIGNMENT_MASK 7

void traceFREE(void *x, size_t s) {
  UNUSED(x);
  UNUSED(s);
}

void traceMALLOC(void *x, size_t s) {
  UNUSED(x);
  UNUSED(s);
}

void mtCOVERAGE_TEST_MARKER() {}

void xTaskResumeAll() {}
void vTaskSuspendAll() {}

#define configASSERT(cond) ASSERT(cond)
