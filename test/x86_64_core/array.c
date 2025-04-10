#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/proxy/value/array.h"
#include "x86_64_core/proxy/value/uint.h"
#include "x86_64_core/value.h"

Test(x86_64_array, test1) {
  x86_64_value value1;
  x86_64_value value2;

  __x86_64_proxy_array_init(&value1, 10);

  value1.op_tbl->op_repr(&value2, &value1);
  debug("array: %s, type %d", value2.data_ptr, value1.type);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_array, test2_index_1d) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  __x86_64_proxy_array_init(&value1, 10);
  __x86_64_proxy_uint_init(&value2, 3);

  value1.op_tbl->op_index(&value3, &value1, &value2, NULL);

  value3.op_tbl->op_repr(&value4, &value3);
  debug("array_index: %s", value4.data_ptr);
  value4.op_tbl->op_drop(&value4);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
}

Test(x86_64_array, test2_index_Nd_slice) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;
  x86_64_value value5;
  x86_64_value value6;
  x86_64_value value7;

  __x86_64_make_int(&value2, 3);
  __x86_64_make_int(&value3, 4);
  __x86_64_make_int(&value4, 5);
  __x86_64_make_int(&value5, 2);

  __x86_64_make_array(&value1, &value2, &value3, &value4, NULL);

  value1.op_tbl->op_index(&value6, &value1, &value5, &value2, NULL);

  value6.op_tbl->op_repr(&value7, &value6);
  debug("array_index: %s", value7.data_ptr);
  value7.op_tbl->op_drop(&value7);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
  value4.op_tbl->op_drop(&value4);
  value5.op_tbl->op_drop(&value5);
  value6.op_tbl->op_drop(&value6);
}

Test(x86_64_array, test3_index_Nd_set) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;
  x86_64_value value5;
  x86_64_value value6;
  x86_64_value value7;
  x86_64_value value8;

  __x86_64_make_int(&value2, 2);
  __x86_64_make_int(&value3, 3);
  __x86_64_make_int(&value4, 4);
  __x86_64_make_int(&value5, 1);
  __x86_64_make_string(&value8, (const uint8_t *)"ACDEF");

  __x86_64_make_array(&value1, &value2, &value3, &value4, NULL);

  value1.op_tbl->op_index_ref(&value6, &value1, &value5, &value5, NULL);

  value6.op_tbl->op_assign(&value6, &value8);

  value1.op_tbl->op_repr(&value7, &value1);
  debug("array_index: %s", value7.data_ptr);
  value7.op_tbl->op_drop(&value7);

  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
  value4.op_tbl->op_drop(&value4);
  value5.op_tbl->op_drop(&value5);
  value6.op_tbl->op_drop(&value6);
  value7.op_tbl->op_drop(&value7);
  value8.op_tbl->op_drop(&value8);
}

Test(x86_64_array, test4_assign) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;
  x86_64_value value5;
  x86_64_value value6;
  x86_64_value value7;
  x86_64_value value8;
  x86_64_value value9;

  __x86_64_make_int(&value2, 2);
  __x86_64_make_int(&value3, 3);
  __x86_64_make_int(&value4, 4);
  __x86_64_make_int(&value5, 1);
  __x86_64_make_string(&value8, (const uint8_t *)"ACDEF");

  __x86_64_make_array(&value1, &value2, &value3, &value4, NULL);

  value1.op_tbl->op_index_ref(&value6, &value1, &value5, &value5, NULL);

  value6.op_tbl->op_assign(&value6, &value8);

  __x86_64_make_void(&value9);
  value9.op_tbl->op_assign(&value9, &value1);

  value1.op_tbl->op_repr(&value7, &value1);
  debug("array_index: %s", value7.data_ptr);
  value7.op_tbl->op_drop(&value7);

  value9.op_tbl->op_repr(&value7, &value9);
  debug("array_index: %s", value7.data_ptr);
  value7.op_tbl->op_drop(&value7);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
  value4.op_tbl->op_drop(&value4);
  value5.op_tbl->op_drop(&value5);
  value6.op_tbl->op_drop(&value6);
  value7.op_tbl->op_drop(&value7);
  value8.op_tbl->op_drop(&value8);
  value9.op_tbl->op_drop(&value9);
}
