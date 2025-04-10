#pragma once

#include "x86_64_core/value.h"

void __x86_64_proxy_array_init(x86_64_value *out, uint64_t length);

x86_64_op_plus        __x86_64_proxy_array_op_plus;
x86_64_op_minus       __x86_64_proxy_array_op_minus;
x86_64_op_not         __x86_64_proxy_array_op_not;
x86_64_op_bit_not     __x86_64_proxy_array_op_bit_not;
x86_64_op_inc         __x86_64_proxy_array_op_inc;
x86_64_op_dec         __x86_64_proxy_array_op_dec;
x86_64_op_or          __x86_64_proxy_array_op_or;
x86_64_op_and         __x86_64_proxy_array_op_and;
x86_64_op_bit_or      __x86_64_proxy_array_op_bit_or;
x86_64_op_bit_xor     __x86_64_proxy_array_op_bit_xor;
x86_64_op_bit_and     __x86_64_proxy_array_op_bit_and;
x86_64_op_eq          __x86_64_proxy_array_op_eq;
x86_64_op_neq         __x86_64_proxy_array_op_neq;
x86_64_op_less        __x86_64_proxy_array_op_less;
x86_64_op_less_eq     __x86_64_proxy_array_op_less_eq;
x86_64_op_bit_shl     __x86_64_proxy_array_op_bit_shl;
x86_64_op_bit_shr     __x86_64_proxy_array_op_bit_shr;
x86_64_op_add         __x86_64_proxy_array_op_add;
x86_64_op_sub         __x86_64_proxy_array_op_sub;
x86_64_op_mul         __x86_64_proxy_array_op_mul;
x86_64_op_div         __x86_64_proxy_array_op_div;
x86_64_op_rem         __x86_64_proxy_array_op_rem;
x86_64_op_index       __x86_64_proxy_array_op_index;
x86_64_op_index_v     __x86_64_proxy_array_op_index_v;
x86_64_op_index_ref   __x86_64_proxy_array_op_index_ref;
x86_64_op_index_ref_v __x86_64_proxy_array_op_index_ref_v;
x86_64_op_member      __x86_64_proxy_array_op_member;
x86_64_op_member_ref  __x86_64_proxy_array_op_member_ref;
x86_64_op_deref       __x86_64_proxy_array_op_deref;
x86_64_op_call        __x86_64_proxy_array_op_call;
x86_64_op_assign      __x86_64_proxy_array_op_assign;
x86_64_op_drop        __x86_64_proxy_array_op_drop;
x86_64_op_copy        __x86_64_proxy_array_op_copy;
x86_64_op_cast        __x86_64_proxy_array_op_cast;
x86_64_op_repr        __x86_64_proxy_array_op_repr;
x86_64_op_type        __x86_64_proxy_array_op_type;
