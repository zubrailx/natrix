#include "value_ref.h"

#include "util/macro.h"
#include "util/strbuf.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value/value_ref.h"

void __x86_64_proxy_value_ref_init(x86_64_value *out, x86_64_value *value) {
  x86_64_data_value_ref data = {.value = value};

  const x86_64_op_tbl *op_tbl =
      X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_VALUE_REF];

  __x86_64_value_init_raw(out, X86_64_TYPE_VALUE_REF, (x86_64_op_tbl *)op_tbl,
                          *(uint64_t *)&data);
}

x86_64_op_plus    __x86_64_proxy_value_ref_op_plus;
x86_64_op_minus   __x86_64_proxy_value_ref_op_minus;
x86_64_op_not     __x86_64_proxy_value_ref_op_not;
x86_64_op_bit_not __x86_64_proxy_value_ref_op_bit_not;
x86_64_op_inc     __x86_64_proxy_value_ref_op_inc;
x86_64_op_dec     __x86_64_proxy_value_ref_op_dec;
x86_64_op_or      __x86_64_proxy_value_ref_op_or;
x86_64_op_and     __x86_64_proxy_value_ref_op_and;
x86_64_op_bit_or  __x86_64_proxy_value_ref_op_bit_or;
x86_64_op_bit_xor __x86_64_proxy_value_ref_op_bit_xor;
x86_64_op_bit_and __x86_64_proxy_value_ref_op_bit_and;
x86_64_op_eq      __x86_64_proxy_value_ref_op_eq;
x86_64_op_neq     __x86_64_proxy_value_ref_op_neq;
x86_64_op_less    __x86_64_proxy_value_ref_op_less;
x86_64_op_less_eq __x86_64_proxy_value_ref_op_less_eq;
x86_64_op_bit_shl __x86_64_proxy_value_ref_op_bit_shl;
x86_64_op_bit_shr __x86_64_proxy_value_ref_op_bit_shr;
x86_64_op_add     __x86_64_proxy_value_ref_op_add;
x86_64_op_sub     __x86_64_proxy_value_ref_op_sub;
x86_64_op_mul     __x86_64_proxy_value_ref_op_mul;
x86_64_op_div     __x86_64_proxy_value_ref_op_div;
x86_64_op_rem     __x86_64_proxy_value_ref_op_rem;

void __x86_64_proxy_value_ref_op_index(x86_64_value *out, x86_64_value *self,
                                       ...) {
  va_list args;
  va_start(args, self);
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_index_v(out, data.value, args);
  va_end(args);
}

void __x86_64_proxy_value_ref_op_index_v(x86_64_value *out, x86_64_value *self,
                                         va_list args) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_index_v(out, data.value, args);
}

void __x86_64_proxy_value_ref_op_index_ref(x86_64_value *out,
                                           x86_64_value *self, ...) {
  va_list args;
  va_start(args, self);
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_index_ref_v(out, data.value, args);
  va_end(args);
}

void __x86_64_proxy_value_ref_op_index_ref_v(x86_64_value *out,
                                             x86_64_value *self, va_list args) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_index_ref_v(out, data.value, args);
}

void __x86_64_proxy_value_ref_op_member(x86_64_value *out, x86_64_value *self,
                                        const uint8_t *member) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_member(out, data.value, member);
}

void __x86_64_proxy_value_ref_op_member_ref(x86_64_value  *out,
                                            x86_64_value  *self,
                                            const uint8_t *member) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_member_ref(out, data.value, member);
}

void __x86_64_proxy_value_ref_op_deref(x86_64_value *out, x86_64_value *self) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  data.value->op_tbl->op_copy(out, data.value);
}

x86_64_op_call __x86_64_proxy_value_ref_op_call;

void __x86_64_proxy_value_ref_op_assign(x86_64_value *self,
                                        x86_64_value *other) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;

  data.value->op_tbl->op_drop(data.value);
  other->op_tbl->op_copy(data.value, other);
}

void __x86_64_proxy_value_ref_op_drop(x86_64_value *self) {
  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_value_ref_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;
  __x86_64_proxy_value_ref_init(out, data.value);
}

x86_64_op_cast __x86_64_proxy_value_ref_op_cast;

void __x86_64_proxy_value_ref_op_repr(x86_64_value *out, x86_64_value *self) {
  x86_64_data_value_ref data = *(x86_64_data_value_ref *)&self->data_raw;

  strbuf *buffer = strbuf_new(64, 0);

  strbuf_append(buffer, "{ref: ");

  x86_64_value value_repr;
  data.value->op_tbl->op_repr(&value_repr, data.value);
  strbuf_append(buffer, value_repr.data_ptr);
  value_repr.op_tbl->op_drop(&value_repr);

  strbuf_append(buffer, "}");

  __x86_64_proxy_string_init_move(out, (uint8_t *)strbuf_detach(buffer));
}

void __x86_64_proxy_value_ref_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"value_ref");
}
