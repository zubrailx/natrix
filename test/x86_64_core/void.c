#include <criterion/criterion.h>
#include <stdint.h>

#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/value.h"

Test(x86_64_void, test1) {
  x86_64_value value;

  __x86_64_make_void(&value);
  value.op_tbl->op_drop(&value);
}

Test(x86_64_void, test2) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_make_void(&value1);
  __x86_64_make_void(&value2);

  value1.op_tbl->op_add(&value3, &value1, &value2);
  value3.op_tbl->op_assign(&value1, &value3);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
}
