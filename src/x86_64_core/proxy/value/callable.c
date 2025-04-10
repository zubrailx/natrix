#include "callable.h"
#include "util/macro.h"
#include "x86_64_core/proxy/registry.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value/callable.h"
#include <stdio.h>

void __x86_64_proxy_callable_init(x86_64_value *out, x86_64_func *func) {
  x86_64_data_callable data = {.func = func};

  const x86_64_op_tbl *op_tbl =
      X86_64_REGISTRY.op_tbl_arr[X86_64_TYPE_CALLABLE];

  __x86_64_value_init_raw(out, X86_64_TYPE_CALLABLE, (x86_64_op_tbl *)op_tbl,
                          *(uint64_t *)&data);
}

x86_64_op_plus        __x86_64_proxy_callable_op_plus;
x86_64_op_minus       __x86_64_proxy_callable_op_minus;
x86_64_op_not         __x86_64_proxy_callable_op_not;
x86_64_op_bit_not     __x86_64_proxy_callable_op_bit_not;
x86_64_op_inc         __x86_64_proxy_callable_op_inc;
x86_64_op_dec         __x86_64_proxy_callable_op_dec;
x86_64_op_or          __x86_64_proxy_callable_op_or;
x86_64_op_and         __x86_64_proxy_callable_op_and;
x86_64_op_bit_or      __x86_64_proxy_callable_op_bit_or;
x86_64_op_bit_xor     __x86_64_proxy_callable_op_bit_xor;
x86_64_op_bit_and     __x86_64_proxy_callable_op_bit_and;
x86_64_op_eq          __x86_64_proxy_callable_op_eq;
x86_64_op_neq         __x86_64_proxy_callable_op_neq;
x86_64_op_less        __x86_64_proxy_callable_op_less;
x86_64_op_less_eq     __x86_64_proxy_callable_op_less_eq;
x86_64_op_bit_shl     __x86_64_proxy_callable_op_bit_shl;
x86_64_op_bit_shr     __x86_64_proxy_callable_op_bit_shr;
x86_64_op_add         __x86_64_proxy_callable_op_add;
x86_64_op_sub         __x86_64_proxy_callable_op_sub;
x86_64_op_mul         __x86_64_proxy_callable_op_mul;
x86_64_op_div         __x86_64_proxy_callable_op_div;
x86_64_op_rem         __x86_64_proxy_callable_op_rem;
x86_64_op_index       __x86_64_proxy_callable_op_index;
x86_64_op_index_v     __x86_64_proxy_callable_op_index_v;
x86_64_op_index_ref   __x86_64_proxy_callable_op_index_ref;
x86_64_op_index_ref_v __x86_64_proxy_callable_op_index_ref_v;
x86_64_op_member      __x86_64_proxy_callable_op_member;
x86_64_op_member_ref  __x86_64_proxy_callable_op_member_ref;
x86_64_op_deref       __x86_64_proxy_callable_op_deref;

x86_64_func *__x86_64_proxy_callable_op_call(x86_64_value *self) {
  x86_64_data_callable data = *(x86_64_data_callable *)&self->data_raw;
  return data.func;
}

void __x86_64_proxy_callable_op_assign(x86_64_value *self,
                                       x86_64_value *other) {
  other->op_tbl->op_copy(self, other);
}

void __x86_64_proxy_callable_op_drop(x86_64_value *self) {
  __x86_64_proxy_void_init(self);
}

void __x86_64_proxy_callable_op_copy(x86_64_value *out, x86_64_value *self) {
  x86_64_data_callable data = *(x86_64_data_callable *)&self->data_raw;
  __x86_64_proxy_callable_init(out, data.func);
}

x86_64_op_cast __x86_64_proxy_callable_op_cast;

void __x86_64_proxy_callable_op_repr(x86_64_value *out, x86_64_value *self) {
  uint8_t buf[64];

  x86_64_data_callable data = *(x86_64_data_callable *)&self->data_raw;

  snprintf((char *)buf, STRMAXLEN(buf), "{callable: 0x%016lx}",
           (uintptr_t)data.func);

  __x86_64_proxy_string_init(out, buf);
}

void __x86_64_proxy_callable_op_type(x86_64_value *out, x86_64_value *self) {
  UNUSED(self);
  __x86_64_proxy_string_init(out, (const uint8_t *)"callable");
}
