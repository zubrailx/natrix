#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/value.h"

Test(x86_64_string, test1) {
  x86_64_value value;

  __x86_64_make_string(&value, (const uint8_t *)"string1");
  value.op_tbl->op_drop(&value);
}

Test(x86_64_string, test2) {
  x86_64_value value1;
  x86_64_value value2;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");
  __x86_64_make_string(&value2, (const uint8_t *)"string2");

  value2.op_tbl->op_assign(&value2, &value1);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_string, test3) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");

  // error
  value1.op_tbl->op_plus(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("%s", value3.data_ptr);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
}

Test(x86_64_string, test4) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");
  __x86_64_make_string(&value2, (const uint8_t *)"string2");

  value1.op_tbl->op_add(&value3, &value1, &value2);

  value3.op_tbl->op_repr(&value4, &value3);
  debug("combined: %s", value4.data_ptr);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
  value4.op_tbl->op_drop(&value4);
}

Test(x86_64_string, test5_assign) {
  x86_64_value value1;
  x86_64_value value2;

  __x86_64_make_string(&value1, (const uint8_t *)"string1");
  __x86_64_make_string(&value2, (const uint8_t *)"string2");

  value1.op_tbl->op_assign(&value1, &value2);
  debug("result: %s", value1.data_ptr);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

// Test(x86_64_string, test5_assign_deref) {
//   x86_64_value value1;
//   x86_64_value value2;
//   x86_64_value value3;

//   __x86_64_make_string(&value1, (const uint8_t *)"string1");
//   __x86_64_make_string(&value2, (const uint8_t *)"string2");

//   value1.op_tbl->op_assign_deref(&value1, &value2);

//   value1.op_tbl->op_repr(&value3, &value1);
//   debug("result: %s", value3.data_ptr);

//   value1.op_tbl->op_drop(&value1);
//   value2.op_tbl->op_drop(&value2);
//   value3.op_tbl->op_drop(&value3);
// }
