#include "test_extern.h"

#include <stdio.h>

uint8_t std_test_first() { return 42; }

void std_test_args(uint8_t a, uint8_t b, int32_t c, uint32_t d, int64_t e,
                   uint64_t f, uint8_t g, const char *h) {
  printf("----------\n");
  printf("bool=%hhu\n", a);
  printf("byte=%hhu\n", b);
  printf("int=%d\n", c);
  printf("uint=%u\n", d);
  printf("long=%ld\n", e);
  printf("ulong=%lu\n", f);
  printf("char=%c\n", g);
  printf("string=%s\n", h);
  printf("----------\n");
}
