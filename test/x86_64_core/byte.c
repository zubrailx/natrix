#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/proxy/value/byte.h"
#include "x86_64_core/value.h"

Test(x86_64_byte, test1) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  __x86_64_proxy_byte_init(&value1, 'a');

  // repr
  value1.op_tbl->op_repr(&value2, &value1);
  debug("byte_repr: %s", value2.data_ptr);
  value2.op_tbl->op_drop(&value2);

  // plus
  value1.op_tbl->op_plus(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_plus_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  // inc
  value1.op_tbl->op_inc(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_inc_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  // dec
  value1.op_tbl->op_dec(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_dec_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  // add
  __x86_64_proxy_byte_init(&value4, 200);

  value4.op_tbl->op_add(&value2, &value4, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_add_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
  value4.op_tbl->op_drop(&value4);
}

Test(x86_64_byte, test2_cast) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_proxy_byte_init(&value1, 'a');

  value1.op_tbl->op_cast(&value2, &value1, X86_64_TYPE_ARRAY);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_cast_array: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_byte, test3_cast) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_proxy_byte_init(&value1, 'a');

  value1.op_tbl->op_cast(&value2, &value1, X86_64_TYPE_LONG);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_cast_long: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_byte, test4_cast) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_proxy_byte_init(&value1, 'a');

  value1.op_tbl->op_cast(&value2, &value1, X86_64_TYPE_CHAR);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_cast_char: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}

Test(x86_64_byte, test5_cast) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_proxy_byte_init(&value1, 'a');

  value1.op_tbl->op_cast(&value2, &value1, X86_64_TYPE_BYTE);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("byte_cast_char: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
}
