#include "builtin.h"

#include <stdio.h>

void __x86_64_print(x86_64_value *self) {
  x86_64_value self_repr;
  x86_64_value self_type;

  self->op_tbl->op_repr(&self_repr, self);
  self->op_tbl->op_type(&self_type, self);

  fprintf(stdout, "%s: %s\n", (char *)self_repr.data_ptr,
          (char *)self_type.data_ptr);

  self_repr.op_tbl->op_drop(&self_repr);
  self_type.op_tbl->op_drop(&self_type);
}
