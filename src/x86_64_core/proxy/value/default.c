#include "default.h"

#include "util/macro.h"
#include "x86_64_core/proxy/util.h"

void __x86_64_proxy_default_op_plus(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "plus");
}

void __x86_64_proxy_default_op_minus(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "minus");
}

void __x86_64_proxy_default_op_not(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "not");
}

void __x86_64_proxy_default_op_bit_not(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "bit_not");
}

void __x86_64_proxy_default_op_inc(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "inc");
}

void __x86_64_proxy_default_op_dec(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "dec");
}

void __x86_64_proxy_default_op_or(x86_64_value *out, x86_64_value *self,
                                  x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "or");
}

void __x86_64_proxy_default_op_and(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "and");
}

void __x86_64_proxy_default_op_bit_or(x86_64_value *out, x86_64_value *self,
                                      x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "bit_or");
}

void __x86_64_proxy_default_op_bit_xor(x86_64_value *out, x86_64_value *self,
                                       x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "bit_xor");
}

void __x86_64_proxy_default_op_bit_and(x86_64_value *out, x86_64_value *self,
                                       x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "bit_and");
}

void __x86_64_proxy_default_op_eq(x86_64_value *out, x86_64_value *self,
                                  x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "eq");
}

void __x86_64_proxy_default_op_neq(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "neq");
}

void __x86_64_proxy_default_op_less(x86_64_value *out, x86_64_value *self,
                                    x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "less");
}

void __x86_64_proxy_default_op_less_eq(x86_64_value *out, x86_64_value *self,
                                       x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "less_eq");
}

void __x86_64_proxy_default_op_bit_shl(x86_64_value *out, x86_64_value *self,
                                       x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "bit_shl");
}

void __x86_64_proxy_default_op_bit_shr(x86_64_value *out, x86_64_value *self,
                                       x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "bit_shr");
}

void __x86_64_proxy_default_op_add(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "add");
}

void __x86_64_proxy_default_op_sub(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "sub");
}

void __x86_64_proxy_default_op_mul(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "mul");
}

void __x86_64_proxy_default_op_div(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "div");
}

void __x86_64_proxy_default_op_rem(x86_64_value *out, x86_64_value *self,
                                   x86_64_value *rsv) {
  UNUSED(rsv);
  __x86_64_proxy_op_error_undefined(out, self, "rem");
}

void __x86_64_proxy_default_op_index(x86_64_value *out, x86_64_value *self,
                                     ...) {
  __x86_64_proxy_op_error_undefined(out, self, "index");
}

void __x86_64_proxy_default_op_index_v(x86_64_value *out, x86_64_value *self,
                                       va_list args) {
  UNUSED(args);
  __x86_64_proxy_op_error_undefined(out, self, "index");
}

void __x86_64_proxy_default_op_index_ref(x86_64_value *out, x86_64_value *self,
                                         ...) {
  __x86_64_proxy_op_error_undefined(out, self, "index_ref");
}

void __x86_64_proxy_default_op_index_ref_v(x86_64_value *out,
                                           x86_64_value *self, va_list args) {
  UNUSED(args);
  __x86_64_proxy_op_error_undefined(out, self, "index_ref");
}

void __x86_64_proxy_default_op_member(x86_64_value *out, x86_64_value *self,
                                      const uint8_t *member) {
  UNUSED(member);
  __x86_64_proxy_op_error_undefined(out, self, "member");
}

void __x86_64_proxy_default_op_member_ref(x86_64_value *out, x86_64_value *self,
                                          const uint8_t *member) {
  UNUSED(member);
  __x86_64_proxy_op_error_undefined(out, self, "member_ref");
}

void __x86_64_proxy_default_op_deref(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "deref");
}

x86_64_func *__x86_64_proxy_default_op_call(x86_64_value *out) {
  UNUSED(out);
  return __x86_64_proxy_op_call_func_error;
}

void __x86_64_proxy_default_op_assign(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "assign");
}

void __x86_64_proxy_default_op_drop(x86_64_value *self) {
  x86_64_value out;
  __x86_64_proxy_op_error_undefined(&out, self, "drop");
  *self = out;
}

void __x86_64_proxy_default_op_copy(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "copy");
}

void __x86_64_proxy_default_op_cast(x86_64_value *out, x86_64_value *self,
                                    x86_64_type_enum type, ...) {
  UNUSED(type);
  __x86_64_proxy_op_error_undefined(out, self, "cast");
}

void __x86_64_proxy_default_op_cast_v(x86_64_value *out, x86_64_value *self,
                                      x86_64_type_enum type, va_list args) {
  UNUSED(type);
  UNUSED(args);
  __x86_64_proxy_op_error_undefined(out, self, "cast");
}

void __x86_64_proxy_default_op_repr(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "repr");
}

void __x86_64_proxy_default_op_type(x86_64_value *out, x86_64_value *self) {
  __x86_64_proxy_op_error_undefined(out, self, "type");
}
