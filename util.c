#include "synchronous_console.h"
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

// Included to satisfy the linker.
int raise(int sig) {
  PANICF("raise(int sig) not implemented. sig=%u", sig);
  // Not reached.
  return 0;
}
