#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/value.h"

Test(x86_64_string_elem_ref, test1_index) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  __x86_64_make_string(&value1, (const uint8_t *)"abcdef");

  __x86_64_make_int(&value2, 3);

  value1.op_tbl->op_index(&value3, &value1, &value2);
  value3.op_tbl->op_repr(&value4, &value3);
  debug("index: %s", value4.data_ptr);
  value4.op_tbl->op_drop(&value4);
  value3.op_tbl->op_drop(&value3);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_string_elem_ref, test2_index_ref) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  __x86_64_make_string(&value1, (const uint8_t *)"abcdef");

  __x86_64_make_int(&value2, 3);
  __x86_64_make_char(&value4, 'D');

  value1.op_tbl->op_index_ref(&value3, &value1, &value2);
  value3.op_tbl->op_assign(&value3, &value4);

  debug("index_ref: %s", value1.data_ptr);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_string_elem_ref, test3_index) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  __x86_64_make_string(&value1, (const uint8_t *)"abcdef");

  __x86_64_make_int(&value2, 3);
  __x86_64_make_string(&value4, (const uint8_t *)"ABCDEF");

  value1.op_tbl->op_index(&value3, &value1, &value2);
  value3.op_tbl->op_assign(&value3, &value4);

  debug("index (should be not changed): %s", value1.data_ptr);
  debug("index (not char, but string): %s", value3.data_ptr);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
  value4.op_tbl->op_drop(&value4);
}

Test(x86_64_string_elem_ref, test3_index_ref_deref) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;
  x86_64_value value5;

  __x86_64_make_string(&value1, (const uint8_t *)"abcdef");

  __x86_64_make_int(&value2, 3);
  __x86_64_make_string(&value4, (const uint8_t *)"ABCDEF");

  value1.op_tbl->op_index_ref(&value3, &value1, &value2);
  value3.op_tbl->op_deref(&value5, &value3);
  value5.op_tbl->op_assign(&value5, &value4);

  debug("index_ref_deref (not char, but string): %s", value1.data_ptr);
  debug("index_ref_deref (not char, but string): %s", value5.data_ptr);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
  value4.op_tbl->op_drop(&value4);
  value5.op_tbl->op_drop(&value5);
}
