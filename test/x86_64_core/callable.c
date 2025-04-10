#include <criterion/criterion.h>
#include <stdarg.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/proxy/value/callable.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/proxy/value/bool.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/value.h"

static void test_func(x86_64_value *ret, ...) {
  va_list args;
  va_start(args, ret);

  x86_64_value *first = va_arg(args, x86_64_value *);

  debug("inside test function: ");
  __x86_64_print(first);

  __x86_64_proxy_int_init(ret, 0);

  va_end(args);
}

Test(x86_64_callable, test1) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;

  __x86_64_proxy_callable_init(&value1, test_func);

  __x86_64_proxy_bool_init(&value2, 12);

  x86_64_func *func = value1.op_tbl->op_call(&value1);
  func(&value3, &value2);

  x86_64_value value4;
  value3.op_tbl->op_repr(&value4, &value3);
  debug("returned: %s", value4.data_ptr);
  value4.op_tbl->op_drop(&value4);

  value1.op_tbl->op_repr(&value4, &value1);
  debug("callable_repr: %s", value4.data_ptr);
  value4.op_tbl->op_drop(&value4);

  value1.op_tbl->op_drop(&value1);
  value2.op_tbl->op_drop(&value2);
  value3.op_tbl->op_drop(&value3);
}
