#include "builtin.h"
#include "util/log.h"

#include "x86_64_core/value/bool.h"
#include "x86_64_core/value/byte.h"
#include "x86_64_core/value/char.h"
#include "x86_64_core/value/int.h"
#include "x86_64_core/value/long.h"
#include "x86_64_core/value/string.h"
#include "x86_64_core/value/uint.h"
#include "x86_64_core/value/ulong.h"
#include <stddef.h>
#include <stdio.h>

static void x86_64_error_type(x86_64_value *out, x86_64_type_enum type) {
  x86_64_value out_repr;
  x86_64_value out_type;

  out->op_tbl->op_repr(&out_repr, out);
  out->op_tbl->op_type(&out_type, out);
  error("value %s: %s(%d) is not of type %d", (char *)out_repr.data_ptr,
        (char *)out_type.data_ptr, out->type, type);
  out_repr.op_tbl->op_drop(&out_repr);
  out_type.op_tbl->op_drop(&out_type);
}

void __x86_64_unwrap_void(x86_64_value *out) {
  if (out->type != X86_64_TYPE_VOID) {
    x86_64_error_type(out, X86_64_TYPE_VOID);
    return;
  }
}

uint8_t __x86_64_unwrap_bool(x86_64_value *out) {
  if (out->type != X86_64_TYPE_BOOL) {
    x86_64_error_type(out, X86_64_TYPE_BOOL);
    return 0;
  }
  x86_64_data_bool data = *(x86_64_data_bool *)&out->data_raw;
  return data.value;
}

uint8_t __x86_64_unwrap_byte(x86_64_value *out) {
  if (out->type != X86_64_TYPE_BYTE) {
    x86_64_error_type(out, X86_64_TYPE_BYTE);
    return 0;
  }
  x86_64_data_byte data = *(x86_64_data_byte *)&out->data_raw;
  return data.value;
}

int32_t __x86_64_unwrap_int(x86_64_value *out) {
  if (out->type != X86_64_TYPE_INT) {
    x86_64_error_type(out, X86_64_TYPE_INT);
    return 0;
  }

  x86_64_data_int data = *(x86_64_data_int *)&out->data_raw;
  return data.value;
}

uint32_t __x86_64_unwrap_uint(x86_64_value *out) {
  if (out->type != X86_64_TYPE_UINT) {
    x86_64_error_type(out, X86_64_TYPE_UINT);
    return 0;
  }

  x86_64_data_uint data = *(x86_64_data_uint *)&out->data_raw;
  return data.value;
}

int64_t __x86_64_unwrap_long(x86_64_value *out) {
  if (out->type != X86_64_TYPE_LONG) {
    x86_64_error_type(out, X86_64_TYPE_LONG);
    return 0;
  }

  x86_64_data_long data = *(x86_64_data_long *)&out->data_raw;
  return data.value;
}

uint64_t __x86_64_unwrap_ulong(x86_64_value *out) {
  if (out->type != X86_64_TYPE_ULONG) {
    x86_64_error_type(out, X86_64_TYPE_ULONG);
    return 0;
  }

  x86_64_data_ulong data = *(x86_64_data_ulong *)&out->data_raw;
  return data.value;
}

uint8_t __x86_64_unwrap_char(x86_64_value *out) {
  if (out->type != X86_64_TYPE_CHAR) {
    x86_64_error_type(out, X86_64_TYPE_CHAR);
    return 0;
  }

  x86_64_data_char data = *(x86_64_data_char *)&out->data_raw;
  return data.value;
}

uint8_t *__x86_64_unwrap_string(x86_64_value *out) {
  if (out->type != X86_64_TYPE_STRING) {
    x86_64_error_type(out, X86_64_TYPE_STRING);
    return NULL;
  }

  x86_64_data_string data = *(x86_64_data_string *)&out->data_raw;
  return data.value;
}
