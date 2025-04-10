#include "builtin.h"
#include "x86_64_core/proxy/util.h"
#include "x86_64_core/proxy/value/array.h"
#include "x86_64_core/proxy/value/bool.h"
#include "x86_64_core/proxy/value/byte.h"
#include "x86_64_core/proxy/value/callable.h"
#include "x86_64_core/proxy/value/char.h"
#include "x86_64_core/proxy/value/error.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/proxy/value/long.h"
#include "x86_64_core/proxy/value/object.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/uint.h"
#include "x86_64_core/proxy/value/ulong.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value/array.h"
#include <stdarg.h>
#include <stdlib.h>

void __x86_64_make_void(x86_64_value *out) { __x86_64_proxy_void_init(out); }

void __x86_64_make_bool(x86_64_value *out, uint8_t value) {
  __x86_64_proxy_bool_init(out, value);
}

void __x86_64_make_byte(x86_64_value *out, uint8_t value) {
  __x86_64_proxy_byte_init(out, value);
}

void __x86_64_make_int(x86_64_value *out, int32_t value) {
  __x86_64_proxy_int_init(out, value);
}

void __x86_64_make_uint(x86_64_value *out, uint32_t value) {
  __x86_64_proxy_uint_init(out, value);
}

void __x86_64_make_long(x86_64_value *out, int64_t value) {
  __x86_64_proxy_long_init(out, value);
}

void __x86_64_make_ulong(x86_64_value *out, uint64_t value) {
  __x86_64_proxy_ulong_init(out, value);
}

void __x86_64_make_char(x86_64_value *out, uint8_t value) {
  __x86_64_proxy_char_init(out, value);
}

void __x86_64_make_string(x86_64_value *out, const uint8_t *value) {
  __x86_64_proxy_string_init(out, value);
}

void __x86_64_make_string_move(x86_64_value *out, uint8_t *value) {
  __x86_64_proxy_string_init_move(out, value);
}

void __x86_64_make_callable(x86_64_value *out, x86_64_func *func) {
  __x86_64_proxy_callable_init(out, func);
}

static void __x86_64_make_array_recursive(x86_64_value *out, uint64_t *stack) {
  uint64_t length = *stack;
  if (!length) {
    return;
  }

  __x86_64_proxy_array_init(out, length);
  x86_64_data_array *data = (x86_64_data_array *)out->data_ptr;

  for (uint64_t i = 0; i < length; i++) {
    __x86_64_make_array_recursive(data->elements + i, stack + 1);
  }
}

void __x86_64_make_array(x86_64_value *out, ...) {
  uint64_t stack_len = 0;

  va_list args;
  va_start(args, out);

  // get args count + validate them
  va_list args_copy;
  va_copy(args_copy, args);

  do {
    x86_64_value *value = va_arg(args_copy, x86_64_value *);
    if (!value) {
      break;
    }

    uint64_t length = __x86_64_proxy_value_as_index(value);
    if (length == UINT64_MAX) {
      __x86_64_proxy_op_error_not_number(out, value, "make");
      goto cleanup;
    }

    ++stack_len;
  } while (1);

  va_end(args_copy);

  if (!stack_len) {
    __x86_64_proxy_op_error_string(out, "make", "no args passed");
    goto cleanup;
  }

  // create stack with array sizes
  uint64_t *stack = malloc(sizeof(uint64_t) * (stack_len + 1));

  for (uint64_t i = 0; i < stack_len; ++i) {
    x86_64_value *value = va_arg(args, x86_64_value *);
    stack[i]            = __x86_64_proxy_value_as_index(value);
  }
  stack[stack_len] = 0;

  __x86_64_make_array_recursive(out, stack);

  free(stack);

cleanup:
  va_end(args);
}

void __x86_64_make_object(x86_64_value                     *out,
                          const x86_64_data_object_symbols *symbols) {
  __x86_64_proxy_object_init(out, symbols);
}

void __x86_64_make_object_setup(x86_64_value                     *out,
                                const x86_64_data_object_symbols *symbols,
                                x86_64_value                     *defaults) {
  __x86_64_proxy_object_init(out, symbols);
  x86_64_data_object *data = (x86_64_data_object *)out->data_ptr;

  for (uint64_t i = 0; i < data->symbols_ref->count; ++i) {
    data->members[i] = defaults[i];
  }
}

void __x86_64_make_error(x86_64_value *out, x86_64_value *value) {
  __x86_64_proxy_error_init(out, value);
}
