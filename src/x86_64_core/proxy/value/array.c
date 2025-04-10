#include "array.h"

#include "util/macro.h"
#include "util/strbuf.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/util.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/ulong.h"
#include "x86_64_core/proxy/value/value_ref.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value/array.h"
#include <stdlib.h>
#include <string.h>

void __x86_64_proxy_array_init(x86_64_value *out, uint64_t length) {
  x86_64_data_array *data =
      malloc(sizeof(x86_64_data_array) + length * sizeof(x86_64_value));

  data->ref_cnt = 1;
  data->length  = length;
  for (uint64_t i = 0; i < length; ++i) {
    __x86_64_proxy_void_init(data->elements + i);
  }

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_ARRAY];

  __x86_64_value_init_ptr(out, X86_64_TYPE_ARRAY, (x86_64_op_tbl *)op_tbl,
                          data);
}

x86_64_op_plus    __x86_64_proxy_array_op_plus;
x86_64_op_minus   __x86_64_proxy_array_op_minus;
x86_64_op_not     __x86_64_proxy_array_op_not;
x86_64_op_bit_not __x86_64_proxy_array_op_bit_not;
x86_64_op_inc     __x86_64_proxy_array_op_inc;
x86_64_op_dec     __x86_64_proxy_array_op_dec;
x86_64_op_or      __x86_64_proxy_array_op_or;
x86_64_op_and     __x86_64_proxy_array_op_and;
x86_64_op_bit_or  __x86_64_proxy_array_op_bit_or;
x86_64_op_bit_xor __x86_64_proxy_array_op_bit_xor;
x86_64_op_bit_and __x86_64_proxy_array_op_bit_and;
x86_64_op_eq      __x86_64_proxy_array_op_eq;
x86_64_op_neq     __x86_64_proxy_array_op_neq;
x86_64_op_less    __x86_64_proxy_array_op_less;
x86_64_op_less_eq __x86_64_proxy_array_op_less_eq;
x86_64_op_bit_shl __x86_64_proxy_array_op_bit_shl;
x86_64_op_bit_shr __x86_64_proxy_array_op_bit_shr;
x86_64_op_add     __x86_64_proxy_array_op_add;
x86_64_op_sub     __x86_64_proxy_array_op_sub;
x86_64_op_mul     __x86_64_proxy_array_op_mul;
x86_64_op_div     __x86_64_proxy_array_op_div;
x86_64_op_rem     __x86_64_proxy_array_op_rem;

void __x86_64_proxy_array_op_index(x86_64_value *out, x86_64_value *self, ...) {
  va_list args;
  va_start(args, self);
  __x86_64_proxy_array_op_index_v(out, self, args);
  va_end(args);
}

void __x86_64_proxy_array_op_index_v(x86_64_value *out, x86_64_value *self,
                                     va_list args) {
  x86_64_value tmp = *self;

  for (x86_64_value *arg = va_arg(args, x86_64_value *); arg;
       arg               = va_arg(args, x86_64_value *)) {

    if (tmp.type != X86_64_TYPE_ARRAY) {
      __x86_64_proxy_op_error_type_mismatch(out, &tmp, "index",
                                            X86_64_TYPE_ARRAY);
      return;
    }

    uint64_t index = __x86_64_proxy_value_as_index(arg);
    if (index == UINT64_MAX) {
      __x86_64_proxy_op_error_not_number(out, arg, "index");
      return;
    }

    x86_64_data_array *data = (x86_64_data_array *)tmp.data_ptr;
    if (index >= data->length) {
      __x86_64_proxy_op_error_string(out, "index", "out of bounds");
      return;
    }

    tmp = data->elements[index];
  }

  tmp.op_tbl->op_copy(out, &tmp);
}

void __x86_64_proxy_array_op_index_ref(x86_64_value *out, x86_64_value *self,
                                       ...) {
  va_list args;
  va_start(args, self);
  __x86_64_proxy_array_op_index_ref_v(out, self, args);
  va_end(args);
}

void __x86_64_proxy_array_op_index_ref_v(x86_64_value *out, x86_64_value *self,
                                         va_list args) {

  x86_64_value tmp = *self;

  x86_64_data_array *data;
  uint64_t           index;

  for (x86_64_value *arg = va_arg(args, x86_64_value *); arg;
       arg               = va_arg(args, x86_64_value *)) {

    if (tmp.type != X86_64_TYPE_ARRAY) {
      __x86_64_proxy_op_error_type_mismatch(out, &tmp, "index_ref",
                                            X86_64_TYPE_ARRAY);
      return;
    }

    index = __x86_64_proxy_value_as_index(arg);
    if (index == UINT64_MAX) {
      __x86_64_proxy_op_error_not_number(out, arg, "index_ref");
      return;
    }

    data = (x86_64_data_array *)tmp.data_ptr;
    if (index >= data->length) {
      __x86_64_proxy_op_error_string(out, "index_ref", "out of bounds");
      return;
    }

    tmp = data->elements[index];
  }

  __x86_64_proxy_value_ref_init(out, data->elements + index);
}

void __x86_64_proxy_array_op_member(x86_64_value *out, x86_64_value *self,
                                    const uint8_t *member) {
  if (!strcmp((const char *)member, "length")) {
    x86_64_data_array *data = (x86_64_data_array *)self->data_ptr;
    __x86_64_proxy_ulong_init(out, data->length);
  } else {
    __x86_64_proxy_op_error_no_member(out, "index", member);
  }
}

x86_64_op_member_ref __x86_64_proxy_array_op_member_ref;
x86_64_op_deref      __x86_64_proxy_array_op_deref;
x86_64_op_call       __x86_64_proxy_array_op_call;

void __x86_64_proxy_array_op_assign(x86_64_value *self, x86_64_value *other) {
  if (self == other) {
    return;
  }
  self->op_tbl->op_drop(self);
  other->op_tbl->op_copy(self, other);
}

void __x86_64_proxy_array_op_drop(x86_64_value *self) {
  x86_64_data_array *data = (x86_64_data_array *)self->data_ptr;

  if (!--data->ref_cnt) {
    for (uint64_t i = 0; i < data->length; ++i) {
      x86_64_value *value = data->elements + i;
      value->op_tbl->op_drop(value);
    }
    free(data);
  }

  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_array_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_array *data = (x86_64_data_array *)self->data_ptr;
  data->ref_cnt += 1;

  *out = *self;
}

x86_64_op_cast __x86_64_proxy_array_op_cast;

void __x86_64_proxy_array_op_repr(x86_64_value *out, x86_64_value *self) {
  strbuf *buffer = strbuf_new(64, 0);

  x86_64_data_array *data = (x86_64_data_array *)self->data_ptr;

  strbuf_append(buffer, "[");

  for (uint64_t i = 0; i < data->length; ++i) {
    if (i != 0) {
      strbuf_append(buffer, ", ");
    }

    x86_64_value elem = data->elements[i];

    x86_64_value elem_string;
    elem.op_tbl->op_repr(&elem_string, &elem);
    strbuf_append(buffer, elem_string.data_ptr);
    elem_string.op_tbl->op_drop(&elem_string);
  }

  strbuf_append(buffer, "]");

  __x86_64_proxy_string_init_move(out, (uint8_t *)strbuf_detach(buffer));
}

void __x86_64_proxy_array_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"<array>");
}
