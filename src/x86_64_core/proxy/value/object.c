#include "object.h"

#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/util.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/value_ref.h"
#include "x86_64_core/proxy/value/void.h"
#include <stdlib.h>
#include <string.h>

void __x86_64_proxy_object_init(x86_64_value                     *out,
                                const x86_64_data_object_symbols *symbols) {

  x86_64_data_object *data = malloc(sizeof(x86_64_data_object) +
                                    symbols->count * sizeof(x86_64_value));
  data->ref_cnt            = 1;
  data->symbols_ref        = symbols;
  for (uint64_t i = 0; i < symbols->count; ++i) {
    __x86_64_proxy_void_init(data->members + i);
  }

  const x86_64_op_tbl *op_tbl = X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_OBJECT];

  __x86_64_value_init_ptr(out, X86_64_TYPE_OBJECT, (x86_64_op_tbl *)op_tbl,
                          data);
}

x86_64_op_plus    __x86_64_proxy_object_op_plus;
x86_64_op_minus   __x86_64_proxy_object_op_minus;
x86_64_op_not     __x86_64_proxy_object_op_not;
x86_64_op_bit_not __x86_64_proxy_object_op_bit_not;
x86_64_op_inc     __x86_64_proxy_object_op_inc;
x86_64_op_dec     __x86_64_proxy_object_op_dec;
x86_64_op_or      __x86_64_proxy_object_op_or;
x86_64_op_and     __x86_64_proxy_object_op_and;
x86_64_op_bit_or  __x86_64_proxy_object_op_bit_or;
x86_64_op_bit_xor __x86_64_proxy_object_op_bit_xor;
x86_64_op_bit_and __x86_64_proxy_object_op_bit_and;
x86_64_op_eq      __x86_64_proxy_object_op_eq;
x86_64_op_neq     __x86_64_proxy_object_op_neq;
x86_64_op_less    __x86_64_proxy_object_op_less;
x86_64_op_less_eq __x86_64_proxy_object_op_less_eq;
x86_64_op_bit_shl __x86_64_proxy_object_op_bit_shl;
x86_64_op_bit_shr __x86_64_proxy_object_op_bit_shr;
x86_64_op_add     __x86_64_proxy_object_op_add;
x86_64_op_sub     __x86_64_proxy_object_op_sub;
x86_64_op_mul     __x86_64_proxy_object_op_mul;
x86_64_op_div     __x86_64_proxy_object_op_div;
x86_64_op_rem     __x86_64_proxy_object_op_rem;

void __x86_64_proxy_object_op_index(x86_64_value *out, x86_64_value *self,
                                    ...) {
  va_list args;
  va_start(args, self);
  __x86_64_proxy_object_op_index_v(out, self, args);
  va_end(args);
}

void __x86_64_proxy_object_op_index_v(x86_64_value *out, x86_64_value *self,
                                      va_list args) {
  x86_64_value *value = va_arg(args, typeof(value));
  if (!value) {
    __x86_64_proxy_op_error_string(out, "index", "op index no arg passed");
    return;
  }

  uint64_t index = __x86_64_proxy_value_as_index(value);
  if (index == UINT64_MAX) {
    __x86_64_proxy_op_error_not_number(out, value, "index");
    return;
  }

  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;

  if (index >= data->symbols_ref->count) {
    __x86_64_proxy_op_error_string(out, "index", "out of bounds");
    return;
  }

  x86_64_value *elem = data->members + index;
  elem->op_tbl->op_copy(out, elem);
}

void __x86_64_proxy_object_op_index_ref(x86_64_value *out, x86_64_value *self,
                                        ...) {
  va_list args;
  va_start(args, self);
  __x86_64_proxy_object_op_index_ref_v(out, self, args);
  va_end(args);
}
void __x86_64_proxy_object_op_index_ref_v(x86_64_value *out, x86_64_value *self,
                                          va_list args) {
  x86_64_value *value = va_arg(args, typeof(value));
  if (!value) {
    __x86_64_proxy_op_error_string(out, "index_ref", "op index no arg passed");
    return;
  }

  uint64_t index = __x86_64_proxy_value_as_index(value);
  if (index == UINT64_MAX) {
    __x86_64_proxy_op_error_not_number(out, value, "index_ref");
    return;
  }

  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;

  if (index >= data->symbols_ref->count) {
    __x86_64_proxy_op_error_string(out, "index_ref", "out of bounds");
    return;
  }

  x86_64_value *elem = data->members + index;

  __x86_64_proxy_value_ref_init(out, elem);
}

void __x86_64_proxy_object_op_member(x86_64_value *out, x86_64_value *self,
                                     const uint8_t *member) {
  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;

  for (uint64_t i = 0; i < data->symbols_ref->count; ++i) {
    const x86_64_data_object_symbol *symbol = data->symbols_ref->symbols + i;

    if (!strcmp((const char *)symbol->name, (const char *)member)) {
      x86_64_value *elem = data->members + i;
      elem->op_tbl->op_copy(out, elem);
      return;
    }
  }

  __x86_64_proxy_op_error_no_member(out, "member", member);
}

void __x86_64_proxy_object_op_member_ref(x86_64_value *out, x86_64_value *self,
                                         const uint8_t *member) {
  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;

  for (uint64_t i = 0; i < data->symbols_ref->count; ++i) {
    const x86_64_data_object_symbol *symbol = data->symbols_ref->symbols + i;

    if (!strcmp((const char *)symbol->name, (const char *)member)) {
      x86_64_value *elem = data->members + i;
      __x86_64_proxy_value_ref_init(out, elem);
      return;
    }
  }

  __x86_64_proxy_op_error_no_member(out, "member_ref", member);
}

x86_64_op_deref __x86_64_proxy_object_op_deref;
x86_64_op_call  __x86_64_proxy_object_op_call;

void __x86_64_proxy_object_op_assign(x86_64_value *self, x86_64_value *other) {
  if (self == other) {
    return;
  }
  self->op_tbl->op_drop(self);
  other->op_tbl->op_copy(self, other);
}

void __x86_64_proxy_object_op_drop(x86_64_value *self) {
  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;

  if (!--data->ref_cnt) {
    for (uint64_t i = 0; i < data->symbols_ref->count; ++i) {
      x86_64_value *value = data->members + i;
      value->op_tbl->op_drop(value);
    }
    free(data);
  }

  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_object_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;
  data->ref_cnt += 1;

  *out = *self;
}

x86_64_op_cast __x86_64_proxy_object_op_cast;

void __x86_64_proxy_object_op_repr(x86_64_value *out, x86_64_value *self) {
  strbuf *buffer = strbuf_new(64, 0);

  strbuf_append(buffer, "{");

  x86_64_data_object *data = (x86_64_data_object *)self->data_ptr;

  for (uint64_t i = 0; i < data->symbols_ref->count; ++i) {
    if (i != 0) {
      strbuf_append(buffer, ", ");
    }

    x86_64_value                    *member = data->members + i;
    const x86_64_data_object_symbol *symbol = data->symbols_ref->symbols + i;

    strbuf_append(buffer, (const char *)symbol->name);
    strbuf_append(buffer, ": ");

    x86_64_value member_string;
    member->op_tbl->op_repr(&member_string, member);
    strbuf_append(buffer, member_string.data_ptr);
    member_string.op_tbl->op_drop(&member_string);
  }

  strbuf_append(buffer, "}");

  __x86_64_proxy_string_init_move(out, (uint8_t *)strbuf_detach(buffer));
}

void __x86_64_proxy_object_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"<object>");
}
