#include "util.h"

#include "util/log.h"
#include "util/strbuf.h"
#include "x86_64_core/proxy/value/error.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/value/byte.h"
#include "x86_64_core/value/char.h"
#include "x86_64_core/value/int.h"
#include "x86_64_core/value/long.h"
#include "x86_64_core/value/uint.h"
#include "x86_64_core/value/ulong.h"

void __x86_64_proxy_op_call_func_error(x86_64_value *ret, ...) {
  const char *err_msg = "op call is not implemented: error noop func is called";

  error(err_msg);

  x86_64_value error_value;
  __x86_64_proxy_string_init(&error_value, (const uint8_t *)err_msg);
  __x86_64_proxy_error_init(ret, &error_value);
}

void __x86_64_proxy_op_error_string(x86_64_value *out, const char *op,
                                    const char *err_msg) {
  strbuf *buffer = strbuf_new(64, 0);

  strbuf_append(buffer, "op ");
  strbuf_append(buffer, op);
  strbuf_append(buffer, " ");
  strbuf_append(buffer, err_msg);

  x86_64_value error_msg;
  __x86_64_proxy_string_init_move(&error_msg, (uint8_t *)strbuf_detach(buffer));

  x86_64_value error;
  __x86_64_proxy_error_init(&error, &error_msg);

  *out = error;
}

void __x86_64_proxy_op_error_undefined(x86_64_value *out, x86_64_value *self,
                                       const char *op) {
  x86_64_value self_repr;
  x86_64_value self_type;
  strbuf      *buffer = strbuf_new(64, 0);

  strbuf_append(buffer, "is undefined for `");

  self->op_tbl->op_repr(&self_repr, self);
  strbuf_append(buffer, (const char *)self_repr.data_ptr);
  self_repr.op_tbl->op_drop(&self_repr);

  strbuf_append(buffer, "`: ");

  self->op_tbl->op_type(&self_type, self);
  strbuf_append(buffer, (const char *)self_type.data_ptr);
  self_type.op_tbl->op_drop(&self_type);

  __x86_64_proxy_op_error_string(out, op, strbuf_data(buffer));

  strbuf_free(buffer);
}

void __x86_64_proxy_op_error_type_mismatch(x86_64_value *out,
                                           x86_64_value *self, const char *op,
                                           x86_64_type_enum type) {
  char         buf[64];
  x86_64_value self_repr;
  x86_64_value self_type;

  strbuf *buffer = strbuf_new(64, 0);

  strbuf_append(buffer, "type mismatch, expected ");
  strbuf_append_f(buffer, buf, "%d", type);
  strbuf_append(buffer, " got `");

  self->op_tbl->op_repr(&self_repr, self);
  strbuf_append(buffer, (char *)self_repr.data_ptr);
  self_repr.op_tbl->op_drop(&self_repr);

  strbuf_append(buffer, "`: ");

  self->op_tbl->op_type(&self_type, self);
  strbuf_append(buffer, (const char *)self_type.data_ptr);
  self_type.op_tbl->op_drop(&self_type);

  strbuf_append_f(buffer, buf, "(%d)", self->type);

  __x86_64_proxy_op_error_string(out, op, strbuf_data(buffer));

  strbuf_free(buffer);
}

void __x86_64_proxy_op_error_unable_to_cast(x86_64_value    *out,
                                            x86_64_value    *self,
                                            x86_64_type_enum type) {
  char         buf[64];
  x86_64_value self_repr;
  x86_64_value self_type;
  strbuf      *buffer = strbuf_new(0, 0);

  strbuf_append(buffer, "unable to cast `");

  self->op_tbl->op_repr(&self_repr, self);
  strbuf_append(buffer, self_repr.data_ptr);
  self_repr.op_tbl->op_drop(&self_repr);

  strbuf_append(buffer, "`: ");

  self->op_tbl->op_type(&self_type, self);
  strbuf_append(buffer, (const char *)self_type.data_ptr);
  self_type.op_tbl->op_drop(&self_type);

  strbuf_append_f(buffer, buf, "(%d)", self->type);

  strbuf_append(buffer, " to type ");
  strbuf_append_f(buffer, buf, "%d", type);

  __x86_64_proxy_op_error_string(out, "cast", strbuf_data(buffer));

  strbuf_free(buffer);
}

void __x86_64_proxy_op_error_not_number(x86_64_value *out, x86_64_value *self,
                                        const char *op) {
  char         buf[64];
  x86_64_value self_repr;
  x86_64_value self_type;
  strbuf      *buffer = strbuf_new(64, 0);

  strbuf_append(buffer, "unable to convert `");

  self->op_tbl->op_repr(&self_repr, self);
  strbuf_append(buffer, (char *)self_repr.data_ptr);
  self_repr.op_tbl->op_drop(&self_repr);

  strbuf_append(buffer, "`: ");

  self->op_tbl->op_type(&self_type, self);
  strbuf_append(buffer, (const char *)self_type.data_ptr);
  self_type.op_tbl->op_drop(&self_type);

  strbuf_append_f(buffer, buf, "(%d)", self->type);

  strbuf_append(buffer, " to number");

  __x86_64_proxy_op_error_string(out, op, strbuf_data(buffer));

  strbuf_free(buffer);
}

void __x86_64_proxy_op_error_no_member(x86_64_value *out, const char *op,
                                       const uint8_t *member) {
  strbuf *buffer = strbuf_new(0, 0);

  strbuf_append(buffer, "member `");
  strbuf_append(buffer, (const char *)member);
  strbuf_append(buffer, "` not present");

  __x86_64_proxy_op_error_string(out, op, strbuf_data(buffer));

  strbuf_free(buffer);
}

uint64_t __x86_64_proxy_value_as_index(const x86_64_value *self) {
  switch (self->type) {
    case X86_64_TYPE_BYTE: {
      x86_64_data_byte data = *(x86_64_data_byte *)&self->data_raw;
      return data.value;
    }
    case X86_64_TYPE_CHAR: {
      x86_64_data_char data = *(x86_64_data_char *)&self->data_raw;
      return data.value;
    }
    case X86_64_TYPE_INT: {
      x86_64_data_int data = *(x86_64_data_int *)&self->data_raw;
      return data.value;
    }
    case X86_64_TYPE_LONG: {
      x86_64_data_long data = *(x86_64_data_long *)&self->data_raw;
      return data.value;
    }
    case X86_64_TYPE_UINT: {
      x86_64_data_uint data = *(x86_64_data_uint *)&self->data_raw;
      return data.value;
    }
    case X86_64_TYPE_ULONG: {
      x86_64_data_ulong data = *(x86_64_data_ulong *)&self->data_raw;
      return data.value;
    }
    case X86_64_TYPE_ARRAY:
    case X86_64_TYPE_BOOL:
    case X86_64_TYPE_ERROR:
    case X86_64_TYPE_OBJECT:
    case X86_64_TYPE_STRING:
    case X86_64_TYPE_STRING_ELEM_REF:
    case X86_64_TYPE_VALUE_REF:
    case X86_64_TYPE_VOID:
    default:
      return UINT64_MAX;
  }
}
