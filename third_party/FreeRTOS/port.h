#pragma once

#include "stdlib.h"

void *pvPortMalloc(size_t s);
void vPortFree(void *p);
size_t xPortGetFreeHeapSize(void);
size_t xPortGetMinimumEverFreeHeapSize(void);
