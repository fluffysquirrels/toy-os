#include "util.h"

void *memset(void *bytes, int ch, int len) {
  unsigned char *p = bytes;
  while(len > 0)
    {
      *p = ch;
      p++;
      len--;
    }
  return(bytes);
}
