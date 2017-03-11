#pragma once

#include "c-utils.h"

void *memset(void *bytes, int ch, int len);

#define DEFINE_KEY_COMPARER(func_name, elt_t, key_t, key_member)\
  int func_name(elt_t *e1, elt_t *e2) {                         \
    key_t k1 = e1->key_member;                                  \
    key_t k2 = e2->key_member;                                  \
    return STANDARD_COMPARE(k1, k2);                            \
  }

#define STANDARD_COMPARE(k1, k2)\
    (k1) < (k2) ? -1            \
  : (k1) > (k2) ? +1            \
  : 0;
