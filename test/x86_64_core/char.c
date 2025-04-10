#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/proxy/value/char.h"
#include "x86_64_core/value.h"

Test(x86_64_char, test1) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_proxy_char_init(&value1, 'a');

  // repr
  value1.op_tbl->op_repr(&value2, &value1);
  debug("char_repr: %s", value2.data_ptr);
  value2.op_tbl->op_drop(&value2);

  // plus
  value1.op_tbl->op_plus(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("char_plus_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  // inc
  value1.op_tbl->op_inc(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("char_inc_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  // dec
  value1.op_tbl->op_dec(&value2, &value1);

  value2.op_tbl->op_repr(&value3, &value2);
  debug("char_dec_repr: %s", value3.data_ptr);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
}
