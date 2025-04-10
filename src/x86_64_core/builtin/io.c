#include "builtin.h"
#include <stdio.h>

void __x86_64_flush() {
  fflush(stdout);
  fflush(stderr);
}
