#include "io.h"

#include "x86_64_core/builtin/builtin.h"
#include <stdio.h>

void std_write(x86_64_value *out, x86_64_value *in) {
  x86_64_value repr;
  in->op_tbl->op_repr(&repr, in);
  printf("%s", (char *)repr.data_ptr);
  repr.op_tbl->op_drop(&repr);

  __x86_64_make_void(out);
}
