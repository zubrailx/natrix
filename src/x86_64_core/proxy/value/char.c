#include "char.h"

#include "util/macro.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/util.h"
#include "x86_64_core/proxy/value/bool.h"
#include "x86_64_core/proxy/value/byte.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/proxy/value/long.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/uint.h"
#include "x86_64_core/proxy/value/ulong.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value/char.h"

void __x86_64_proxy_char_init(x86_64_value *out, uint8_t value) {
  x86_64_data_char data = {.value = value};

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_CHAR];

  __x86_64_value_init_raw(out, X86_64_TYPE_CHAR, (x86_64_op_tbl *)op_tbl,
                          *(uint64_t *)&data);
}

void __x86_64_proxy_char_op_plus(x86_64_value *out, x86_64_value *self) {
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_char_op_minus(x86_64_value *out, x86_64_value *self) {
  x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
  __x86_64_proxy_char_init(out, -data.value);
}

x86_64_op_not __x86_64_proxy_char_op_not;

void __x86_64_proxy_char_op_bit_not(x86_64_value *out, x86_64_value *self) {
  x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
  __x86_64_proxy_char_init(out, ~data.value);
}

void __x86_64_proxy_char_op_inc(x86_64_value *out, x86_64_value *self) {
  x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
  __x86_64_proxy_char_init(out, data.value + 1);
}

void __x86_64_proxy_char_op_dec(x86_64_value *out, x86_64_value *self) {
  x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
  __x86_64_proxy_char_init(out, data.value - 1);
}

x86_64_op_or  __x86_64_proxy_char_op_or;
x86_64_op_and __x86_64_proxy_char_op_and;

void __x86_64_proxy_char_op_bit_or(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "bit_or", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value | rsv_data.value);
}

void __x86_64_proxy_char_op_bit_xor(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "bit_xor", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value ^ rsv_data.value);
}

void __x86_64_proxy_char_op_bit_and(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "bit_and", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value & rsv_data.value);
}

void __x86_64_proxy_char_op_eq(x86_64_value *out, x86_64_value *self,
                               x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "eq", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_bool_init(out, self_data.value == rsv_data.value);
}

void __x86_64_proxy_char_op_neq(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "neq", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_bool_init(out, self_data.value != rsv_data.value);
}

void __x86_64_proxy_char_op_less(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "less", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_bool_init(out, self_data.value < rsv_data.value);
}

void __x86_64_proxy_char_op_less_eq(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "less_eq", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_bool_init(out, self_data.value <= rsv_data.value);
}

void __x86_64_proxy_char_op_bit_shl(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "bit_shl", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value << rsv_data.value);
}

void __x86_64_proxy_char_op_bit_shr(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "bit_shr", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value >> rsv_data.value);
}

void __x86_64_proxy_char_op_add(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "add", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value + rsv_data.value);
}

void __x86_64_proxy_char_op_sub(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "sub", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value - rsv_data.value);
}

void __x86_64_proxy_char_op_mul(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "mul", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value * rsv_data.value);
}

void __x86_64_proxy_char_op_div(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "div", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value / rsv_data.value);
}

void __x86_64_proxy_char_op_rem(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "rem", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_char self_data = *(x86_64_data_char *)&self->data_raw;
  x86_64_data_char rsv_data  = *(x86_64_data_char *)&rsv->data_raw;

  __x86_64_proxy_char_init(out, self_data.value % rsv_data.value);
}

x86_64_op_index       __x86_64_proxy_char_op_index;
x86_64_op_index_v     __x86_64_proxy_char_op_index_v;
x86_64_op_index_ref   __x86_64_proxy_char_op_index_ref;
x86_64_op_index_ref_v __x86_64_proxy_char_op_index_ref_v;
x86_64_op_member      __x86_64_proxy_char_op_member;
x86_64_op_member_ref  __x86_64_proxy_char_op_member_ref;
x86_64_op_deref       __x86_64_proxy_char_op_deref;
x86_64_op_call        __x86_64_proxy_char_op_call;

void __x86_64_proxy_char_op_assign(x86_64_value *self, x86_64_value *other) {
  other->op_tbl->op_copy(self, other);
}

void __x86_64_proxy_char_op_drop(x86_64_value *self) {
  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_char_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
  __x86_64_proxy_char_init(out, data.value);
}

void __x86_64_proxy_char_op_cast(x86_64_value *out, x86_64_value *self,
                                 x86_64_type_enum type, ...) {
  switch (type) {
    case X86_64_TYPE_BOOL: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      __x86_64_proxy_bool_init(out, data.value);
      break;
    }
    case X86_64_TYPE_BYTE: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      __x86_64_proxy_byte_init(out, data.value);
      break;
    }
    case X86_64_TYPE_CHAR: {
      self->op_tbl->op_copy(out, self);
      break;
    }
    case X86_64_TYPE_INT: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      __x86_64_proxy_int_init(out, data.value);
      break;
    }
    case X86_64_TYPE_LONG: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      __x86_64_proxy_long_init(out, data.value);
      break;
    }
    case X86_64_TYPE_UINT: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      __x86_64_proxy_uint_init(out, data.value);
      break;
    }
    case X86_64_TYPE_ULONG: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      __x86_64_proxy_ulong_init(out, data.value);
      break;
    }
    case X86_64_TYPE_VOID: {
      __x86_64_proxy_void_init(out);
      break;
    }
    case X86_64_TYPE_ARRAY:
    case X86_64_TYPE_ERROR:
    case X86_64_TYPE_OBJECT:
    case X86_64_TYPE_STRING:
    case X86_64_TYPE_STRING_ELEM_REF:
    case X86_64_TYPE_VALUE_REF:
    default: {
      __x86_64_proxy_op_error_unable_to_cast(out, self, type);
      break;
    }
  }
}

void __x86_64_proxy_char_op_repr(x86_64_value *out, x86_64_value *self) {
  uint8_t buf[2];

  x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
  buf[0]                = data.value;
  buf[1]                = '\0';
  __x86_64_proxy_string_init(out, buf);
}

void __x86_64_proxy_char_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"char");
}
