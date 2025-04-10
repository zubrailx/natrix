#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/value.h"

Test(x86_64_error, test1) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");

  x86_64_value value11;
  value1.op_tbl->op_copy(&value11, &value1);
  __x86_64_make_error(&value2, &value11);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("error_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);

  value1.op_tbl->op_drop(&value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("error_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);

  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_error, test3) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");
  __x86_64_make_error(&value2, &value1);

  value2.op_tbl->op_plus(&value3, &value2);

  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_error, test4_call) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");
  __x86_64_make_error(&value2, &value1);

  x86_64_func *func = value2.op_tbl->op_call(&value2);

  func(&value3);

  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);
}
