#include <criterion/criterion.h>
#include <limits.h>
#include <stdint.h>

#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/proxy/value/bool.h"
#include "x86_64_core/proxy/value/byte.h"
#include "x86_64_core/proxy/value/char.h"
#include "x86_64_core/proxy/value/error.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/proxy/value/long.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/uint.h"
#include "x86_64_core/proxy/value/ulong.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value.h"

Test(x86_64_print, test1_bool) {
  x86_64_value value1;

  __x86_64_proxy_bool_init(&value1, 100);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_byte) {
  x86_64_value value1;

  __x86_64_proxy_byte_init(&value1, 100);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_char) {
  x86_64_value value1;

  __x86_64_proxy_char_init(&value1, 100);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_error) {
  x86_64_value value1;
  x86_64_value value11;

  __x86_64_proxy_string_init(&value11, (const uint8_t *)"error message");
  __x86_64_proxy_error_init(&value1, &value11);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_int) {
  x86_64_value value1;

  __x86_64_proxy_int_init(&value1, -150);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_uint) {
  x86_64_value value1;

  __x86_64_proxy_uint_init(&value1, -150);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_ulong) {
  x86_64_value value1;

  __x86_64_proxy_ulong_init(&value1, -150);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_long) {
  x86_64_value value1;

  __x86_64_proxy_long_init(&value1, -150000000);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_void) {
  x86_64_value value1;

  __x86_64_proxy_void_init(&value1);
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_print, test1_string) {
  x86_64_value value1;

  __x86_64_proxy_string_init(&value1, (const uint8_t *)"some string");
  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
}
