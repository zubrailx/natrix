#pragma once

#include <stdarg.h>
#include <stdint.h>

typedef struct __attribute__((packed)) x86_64_value_struct x86_64_value;

typedef enum x86_64_type_enum {
  X86_64_TYPE_ARRAY,
  X86_64_TYPE_BOOL,
  X86_64_TYPE_BYTE,
  X86_64_TYPE_CALLABLE,
  X86_64_TYPE_CHAR,
  X86_64_TYPE_ERROR,
  X86_64_TYPE_INT,
  X86_64_TYPE_LONG,
  X86_64_TYPE_OBJECT,
  X86_64_TYPE_STRING,
  X86_64_TYPE_STRING_ELEM_REF,
  X86_64_TYPE_UINT,
  X86_64_TYPE_ULONG,
  X86_64_TYPE_VALUE_REF,
  X86_64_TYPE_VOID,
} x86_64_type_enum;

typedef void x86_64_func(x86_64_value *ret, ...);

// unary
typedef void x86_64_op_plus(x86_64_value *out, x86_64_value *self);
typedef void x86_64_op_minus(x86_64_value *out, x86_64_value *self);
typedef void x86_64_op_not(x86_64_value *out, x86_64_value *self);
typedef void x86_64_op_bit_not(x86_64_value *out, x86_64_value *self);
typedef void x86_64_op_inc(x86_64_value *out, x86_64_value *self);
typedef void x86_64_op_dec(x86_64_value *out, x86_64_value *self);
// binary
typedef void x86_64_op_or(x86_64_value *out, x86_64_value *self,
                          x86_64_value *rsv);
typedef void x86_64_op_and(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
typedef void x86_64_op_bit_or(x86_64_value *out, x86_64_value *self,
                              x86_64_value *rsv);
typedef void x86_64_op_bit_xor(x86_64_value *out, x86_64_value *self,
                               x86_64_value *rsv);
typedef void x86_64_op_bit_and(x86_64_value *out, x86_64_value *self,
                               x86_64_value *rsv);
typedef void x86_64_op_eq(x86_64_value *out, x86_64_value *self,
                          x86_64_value *rsv);
typedef void x86_64_op_neq(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
typedef void x86_64_op_less(x86_64_value *out, x86_64_value *self,
                            x86_64_value *rsv);
typedef void x86_64_op_less_eq(x86_64_value *out, x86_64_value *self,
                               x86_64_value *rsv);
typedef void x86_64_op_bit_shl(x86_64_value *out, x86_64_value *self,
                               x86_64_value *rsv);
typedef void x86_64_op_bit_shr(x86_64_value *out, x86_64_value *self,
                               x86_64_value *rsv);
typedef void x86_64_op_add(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
typedef void x86_64_op_sub(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
typedef void x86_64_op_mul(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
typedef void x86_64_op_div(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
typedef void x86_64_op_rem(x86_64_value *out, x86_64_value *self,
                           x86_64_value *rsv);
// access
typedef void x86_64_op_index(x86_64_value *out, x86_64_value *self, ...);
typedef void x86_64_op_index_v(x86_64_value *out, x86_64_value *self,
                               va_list args);
typedef void x86_64_op_index_ref(x86_64_value *out, x86_64_value *self, ...);
typedef void x86_64_op_index_ref_v(x86_64_value *out, x86_64_value *self,
                                   va_list args);
typedef void x86_64_op_member(x86_64_value *out, x86_64_value *self,
                              const uint8_t *member);
typedef void x86_64_op_member_ref(x86_64_value *out, x86_64_value *self,
                                  const uint8_t *member);
typedef void x86_64_op_deref(x86_64_value *out, x86_64_value *self);
// call (returns function that needs to be called)
typedef x86_64_func *x86_64_op_call(x86_64_value *self);
// assignment
typedef void x86_64_op_assign(x86_64_value *self, x86_64_value *other);
// lifetime
typedef void x86_64_op_drop(x86_64_value *out);
typedef void x86_64_op_copy(x86_64_value *out, x86_64_value *self);
// conversion
typedef void x86_64_op_cast(x86_64_value *out, x86_64_value *self,
                            x86_64_type_enum type, ...);
typedef void x86_64_op_cast_v(x86_64_value *out, x86_64_value *self,
                              x86_64_type_enum type, va_list args);
typedef void x86_64_op_repr(x86_64_value *out, x86_64_value *self);
typedef void x86_64_op_type(x86_64_value *out, x86_64_value *self);

typedef struct __attribute__((packed)) x86_64_op_tbl_struct {
  x86_64_op_plus        *op_plus;
  x86_64_op_minus       *op_minus;
  x86_64_op_not         *op_not;
  x86_64_op_bit_not     *op_bit_not;
  x86_64_op_inc         *op_inc;
  x86_64_op_dec         *op_dec;
  x86_64_op_or          *op_or;
  x86_64_op_and         *op_and;
  x86_64_op_bit_or      *op_bit_or;
  x86_64_op_bit_xor     *op_bit_xor;
  x86_64_op_bit_and     *op_bit_and;
  x86_64_op_eq          *op_eq;
  x86_64_op_neq         *op_neq;
  x86_64_op_less        *op_less;
  x86_64_op_less_eq     *op_less_eq;
  x86_64_op_bit_shl     *op_bit_shl;
  x86_64_op_bit_shr     *op_bit_shr;
  x86_64_op_add         *op_add;
  x86_64_op_sub         *op_sub;
  x86_64_op_mul         *op_mul;
  x86_64_op_div         *op_div;
  x86_64_op_rem         *op_rem;
  x86_64_op_index       *op_index;
  x86_64_op_index_v     *op_index_v;
  x86_64_op_index_ref   *op_index_ref;
  x86_64_op_index_ref_v *op_index_ref_v;
  x86_64_op_member      *op_member;
  x86_64_op_member_ref  *op_member_ref;
  x86_64_op_deref       *op_deref;
  x86_64_op_call        *op_call;
  x86_64_op_assign      *op_assign;
  x86_64_op_drop        *op_drop;
  x86_64_op_copy        *op_copy;
  x86_64_op_cast        *op_cast;
  x86_64_op_repr        *op_repr;
  x86_64_op_type        *op_type;
} x86_64_op_tbl;

typedef struct __attribute__((packed)) x86_64_value_struct {
  struct {
    x86_64_type_enum type;
    uint8_t          type_pad[4];
  };
  x86_64_op_tbl *op_tbl;
  union {
    void    *data_ptr;
    uint64_t data_raw;
  };
} x86_64_value;

static inline void __x86_64_value_init_ptr(x86_64_value    *value,
                                           x86_64_type_enum type,
                                           x86_64_op_tbl *op_tbl, void *data) {
  value->type     = type;
  value->op_tbl   = op_tbl;
  value->data_ptr = data;
}

static inline void __x86_64_value_init_raw(x86_64_value    *value,
                                           x86_64_type_enum type,
                                           x86_64_op_tbl   *op_tbl,
                                           uint64_t         data) {
  value->type     = type;
  value->op_tbl   = op_tbl;
  value->data_raw = data;
}
