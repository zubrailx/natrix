#include "error.h"

#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/util.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value/error.h"

#include <stdlib.h>

static void x86_64_proxy_error_unhandled(x86_64_value *self, const char *op) {
  x86_64_value repr;
  self->op_tbl->op_repr(&repr, self);

  error("unhandled error in op %s: %s", op, repr.data_ptr);

  repr.op_tbl->op_drop(&repr);
}

void __x86_64_proxy_error_init(x86_64_value *out, x86_64_value *value) {
  x86_64_data_error *data = malloc(sizeof(x86_64_data_error));
  data->value             = *value;

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_ERROR];

  __x86_64_value_init_ptr(out, X86_64_TYPE_ERROR, (x86_64_op_tbl *)op_tbl,
                          data);
}

// NOTE: currently first error is passed next (propageted)
void __x86_64_proxy_error_op_plus(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "plus");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_minus(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "minus");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_not(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "not");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_bit_not(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "bit_not");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_inc(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "inc");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_dec(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "dec");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_or(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "or");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_and(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "and");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_bit_or(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "bit_or");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_bit_xor(x86_64_value *out, x86_64_value *self,
                                     x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "bit_xor");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_bit_and(x86_64_value *out, x86_64_value *self,
                                     x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "bit_and");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_eq(x86_64_value *out, x86_64_value *self,
                                x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "eq");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_neq(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "neq");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_less(x86_64_value *out, x86_64_value *self,
                                  x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "less");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_less_eq(x86_64_value *out, x86_64_value *self,
                                     x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "less_eq");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_bit_shl(x86_64_value *out, x86_64_value *self,
                                     x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "bit_shl");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_bit_shr(x86_64_value *out, x86_64_value *self,
                                     x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "bit_shr");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_add(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "add");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_sub(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "sub");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_mul(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "mul");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_div(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "div");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_rem(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  UNUSED(rsv);
  x86_64_proxy_error_unhandled(self, "rem");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_index(x86_64_value *out, x86_64_value *self, ...) {
  x86_64_proxy_error_unhandled(self, "index");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_index_v(x86_64_value *out, x86_64_value *self,
                                     va_list args) {
  UNUSED(args);
  x86_64_proxy_error_unhandled(self, "index");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_index_ref(x86_64_value *out, x86_64_value *self,
                                       ...) {
  x86_64_proxy_error_unhandled(self, "index_ref");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_index_ref_v(x86_64_value *out, x86_64_value *self,
                                         va_list args) {
  UNUSED(args);
  x86_64_proxy_error_unhandled(self, "index_ref");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_member(x86_64_value *out, x86_64_value *self,
                                    const uint8_t *member) {
  UNUSED(member);
  x86_64_proxy_error_unhandled(self, "member");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_member_ref(x86_64_value *out, x86_64_value *self,
                                        const uint8_t *member) {
  UNUSED(member);
  x86_64_proxy_error_unhandled(self, "member_ref");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_deref(x86_64_value *out, x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "deref");
  self->op_tbl->op_copy(out, self);
}

x86_64_func *__x86_64_proxy_error_op_call(x86_64_value *self) {
  x86_64_proxy_error_unhandled(self, "call");
  return __x86_64_proxy_op_call_func_error;
}

void __x86_64_proxy_error_op_assign(x86_64_value *self, x86_64_value *other) {
  if (self == other) {
    return;
  }
  self->op_tbl->op_drop(self);
  other->op_tbl->op_copy(self, other);
}

void __x86_64_proxy_error_op_drop(x86_64_value *self) {
  x86_64_data_error *data = (x86_64_data_error *)self->data_ptr;
  data->value.op_tbl->op_drop(&data->value);
  free(data);
  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_error_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_error *data = (x86_64_data_error *)self->data_ptr;
  x86_64_value       data_value;
  data->value.op_tbl->op_copy(&data_value, &data->value);
  __x86_64_proxy_error_init(out, &data_value);
}

void __x86_64_proxy_error_op_cast(x86_64_value *out, x86_64_value *self,
                                  x86_64_type_enum type, ...) {
  UNUSED(type);
  x86_64_proxy_error_unhandled(self, "cast");
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_error_op_repr(x86_64_value *out, x86_64_value *self) {
  strbuf            *buffer = strbuf_new(64, 0);
  x86_64_data_error *data   = (x86_64_data_error *)self->data_ptr;

  x86_64_value value_string;
  data->value.op_tbl->op_repr(&value_string, &data->value);

  strbuf_append(buffer, "{ error: ");
  strbuf_append(buffer, value_string.data_ptr);

  value_string.op_tbl->op_drop(&value_string);
  strbuf_append(buffer, " }");

  __x86_64_proxy_string_init_move(out, (uint8_t *)strbuf_detach(buffer));
}

void __x86_64_proxy_error_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"error");
}
