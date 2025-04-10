#include "string.h"

#include "util/macro.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/util.h"
#include "x86_64_core/proxy/value/bool.h"
#include "x86_64_core/proxy/value/char.h"
#include "x86_64_core/proxy/value/string_elem_ref.h"
#include "x86_64_core/proxy/value/ulong.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value.h"
#include "x86_64_core/value/string.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void __x86_64_proxy_string_init(x86_64_value *out, const uint8_t *value) {
  x86_64_data_string data = {.value = (uint8_t *)strdup((const char *)value)};

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_STRING];

  __x86_64_value_init_raw(out, X86_64_TYPE_STRING, (x86_64_op_tbl *)op_tbl,
                          *(uint64_t *)&data);
}

void __x86_64_proxy_string_init_move(x86_64_value *out, uint8_t *value) {
  x86_64_data_string data = {.value = value};

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_STRING];

  __x86_64_value_init_raw(out, X86_64_TYPE_STRING, (x86_64_op_tbl *)op_tbl,
                          *(uint64_t *)&data);
}

x86_64_op_plus    __x86_64_proxy_string_op_plus;
x86_64_op_minus   __x86_64_proxy_string_op_minus;
x86_64_op_not     __x86_64_proxy_string_op_not;
x86_64_op_bit_not __x86_64_proxy_string_op_bit_not;
x86_64_op_inc     __x86_64_proxy_string_op_inc;
x86_64_op_dec     __x86_64_proxy_string_op_dec;
x86_64_op_or      __x86_64_proxy_string_op_or;
x86_64_op_and     __x86_64_proxy_string_op_and;
x86_64_op_bit_or  __x86_64_proxy_string_op_bit_or;
x86_64_op_bit_xor __x86_64_proxy_string_op_bit_xor;
x86_64_op_bit_and __x86_64_proxy_string_op_bit_and;

void __x86_64_proxy_string_op_eq(x86_64_value *out, x86_64_value *self,
                                 x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "eq", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_string self_data = *(x86_64_data_string *)&self->data_raw;
  x86_64_data_string rsv_data  = *(x86_64_data_string *)&rsv->data_raw;

  int c = strcmp((const char *)self_data.value, (const char *)rsv_data.value);
  return __x86_64_proxy_bool_init(out, !c);
}

void __x86_64_proxy_string_op_neq(x86_64_value *out, x86_64_value *self,
                                  x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "neq", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_string self_data = *(x86_64_data_string *)&self->data_raw;
  x86_64_data_string rsv_data  = *(x86_64_data_string *)&rsv->data_raw;

  int c = strcmp((const char *)self_data.value, (const char *)rsv_data.value);
  return __x86_64_proxy_bool_init(out, c);
}

void __x86_64_proxy_string_op_less(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "less", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_string self_data = *(x86_64_data_string *)&self->data_raw;
  x86_64_data_string rsv_data  = *(x86_64_data_string *)&rsv->data_raw;

  int c = strcmp((const char *)self_data.value, (const char *)rsv_data.value);
  return __x86_64_proxy_bool_init(out, c < 0 ? 1 : 0);
}

void __x86_64_proxy_string_op_less_eq(x86_64_value *out, x86_64_value *self,
                                      x86_64_value *rsv) {
  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "less_eq", self->type);
    __x86_64_proxy_void_init(out);
    return;
  }

  x86_64_data_string self_data = *(x86_64_data_string *)&self->data_raw;
  x86_64_data_string rsv_data  = *(x86_64_data_string *)&rsv->data_raw;

  int c = strcmp((const char *)self_data.value, (const char *)rsv_data.value);
  return __x86_64_proxy_bool_init(out, c <= 0 ? 1 : 0);
}

x86_64_op_bit_shl __x86_64_proxy_string_op_bit_shl;
x86_64_op_bit_shr __x86_64_proxy_string_op_bit_shr;

void __x86_64_proxy_string_op_add(x86_64_value *out, x86_64_value *self,
                                  x86_64_value *rsv) {

  if (self->type != rsv->type) {
    __x86_64_proxy_op_error_type_mismatch(out, rsv, "add", self->type);
    return;
  }

  x86_64_data_string self_data = *(x86_64_data_string *)&self->data_raw;
  x86_64_data_string rsv_data  = *(x86_64_data_string *)&rsv->data_raw;

  size_t self_size = strlen((char *)self_data.value);
  size_t rsv_size  = strlen((char *)rsv_data.value);

  x86_64_data_string data = {.value =
                                 (uint8_t *)malloc(self_size + rsv_size + 1)};

  memcpy(data.value, self_data.value, self_size);
  memcpy(data.value + self_size, rsv_data.value, rsv_size);
  data.value[self_size + rsv_size] = '\0';

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_STRING];

  __x86_64_value_init_raw(out, X86_64_TYPE_STRING, (x86_64_op_tbl *)op_tbl,
                          *(uint64_t *)&data);
}

x86_64_op_sub __x86_64_proxy_string_op_sub;
x86_64_op_mul __x86_64_proxy_string_op_mul;
x86_64_op_div __x86_64_proxy_string_op_div;
x86_64_op_rem __x86_64_proxy_string_op_rem;

void __x86_64_proxy_string_op_index(x86_64_value *out, x86_64_value *self,
                                    ...) {
  va_list args;
  va_start(args, self);
  __x86_64_proxy_string_op_index_v(out, self, args);
  va_end(args);
}

void __x86_64_proxy_string_op_index_v(x86_64_value *out, x86_64_value *self,
                                      va_list args) {
  x86_64_value *arg = va_arg(args, x86_64_value *);
  if (!arg) {
    __x86_64_proxy_op_error_string(out, "index", "op index no arg passed");
    return;
  }

  uint64_t index = __x86_64_proxy_value_as_index(arg);
  if (index == UINT64_MAX) {
    __x86_64_proxy_op_error_not_number(out, arg, "index");
    return;
  }

  x86_64_data_string data = *(x86_64_data_string *)&self->data_raw;

  __x86_64_proxy_char_init(out, data.value[index]);
}

void __x86_64_proxy_string_op_index_ref(x86_64_value *out, x86_64_value *self,
                                        ...) {
  va_list args;
  va_start(args, self);
  __x86_64_proxy_string_op_index_ref_v(out, self, args);
  va_end(args);
}

void __x86_64_proxy_string_op_index_ref_v(x86_64_value *out, x86_64_value *self,
                                          va_list args) {
  x86_64_value *arg = va_arg(args, x86_64_value *);
  if (!arg) {
    __x86_64_proxy_op_error_string(out, "index_ref",
                                   "op index_ref no arg passed");
    return;
  }

  uint64_t index = __x86_64_proxy_value_as_index(arg);
  if (index == UINT64_MAX) {
    __x86_64_proxy_op_error_not_number(out, arg, "index_ref");
    return;
  }

  x86_64_data_string data = *(x86_64_data_string *)&self->data_raw;

  __x86_64_proxy_string_elem_ref_init(out, data.value + index);
}

void __x86_64_proxy_string_op_member(x86_64_value *out, x86_64_value *self,
                                     const uint8_t *member) {
  x86_64_data_string data = *(x86_64_data_string *)&self->data_raw;

  if (!strcmp((const char *)member, "length")) {
    uint64_t length = strlen((const char *)data.value);
    __x86_64_proxy_ulong_init(out, length);
  } else {
    __x86_64_proxy_op_error_no_member(out, "index", member);
  }
}

x86_64_op_member_ref __x86_64_proxy_string_op_member_ref;
x86_64_op_deref      __x86_64_proxy_string_op_deref;
x86_64_op_call       __x86_64_proxy_string_op_call;

void __x86_64_proxy_string_op_assign(x86_64_value *self, x86_64_value *other) {
  if (self == other) {
    return;
  }
  self->op_tbl->op_drop(self);
  other->op_tbl->op_copy(self, other);
}

void __x86_64_proxy_string_op_drop(x86_64_value *self) {
  x86_64_data_string data = *(x86_64_data_string *)&self->data_raw;
  free(data.value);
  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_string_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_string data = *(x86_64_data_string *)&self->data_raw;
  __x86_64_proxy_string_init(out, data.value);
}

x86_64_op_cast __x86_64_proxy_string_op_cast;

void __x86_64_proxy_string_op_repr(x86_64_value *out, x86_64_value *self) {
  self->op_tbl->op_copy(out, self);
}

void __x86_64_proxy_string_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"string");
}
