#include "registry.h"

#include "x86_64_core/proxy/value/array.h"
#include "x86_64_core/proxy/value/bool.h"
#include "x86_64_core/proxy/value/byte.h"
#include "x86_64_core/proxy/value/callable.h"
#include "x86_64_core/proxy/value/char.h"
#include "x86_64_core/proxy/value/default.h"
#include "x86_64_core/proxy/value/error.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/proxy/value/long.h"
#include "x86_64_core/proxy/value/object.h"
#include "x86_64_core/proxy/value/string.h"
#include "x86_64_core/proxy/value/string_elem_ref.h"
#include "x86_64_core/proxy/value/uint.h"
#include "x86_64_core/proxy/value/ulong.h"
#include "x86_64_core/proxy/value/value_ref.h"
#include "x86_64_core/proxy/value/void.h"

static const x86_64_op_tbl X86_64_OP_TBL_ARRAY = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_default_op_eq,
    .op_neq         = __x86_64_proxy_default_op_neq,
    .op_less        = __x86_64_proxy_default_op_less,
    .op_less_eq     = __x86_64_proxy_default_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_default_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_array_op_index,
    .op_index_v     = __x86_64_proxy_array_op_index_v,
    .op_index_ref   = __x86_64_proxy_array_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_array_op_index_ref_v,
    .op_member      = __x86_64_proxy_array_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_array_op_assign,
    .op_drop        = __x86_64_proxy_array_op_drop,
    .op_copy        = __x86_64_proxy_array_op_copy,
    .op_cast        = __x86_64_proxy_default_op_cast,
    .op_repr        = __x86_64_proxy_array_op_repr,
    .op_type        = __x86_64_proxy_array_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_BOOL = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_bool_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_bool_op_or,
    .op_and         = __x86_64_proxy_bool_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_bool_op_eq,
    .op_neq         = __x86_64_proxy_bool_op_neq,
    .op_less        = __x86_64_proxy_default_op_less,
    .op_less_eq     = __x86_64_proxy_default_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_default_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_bool_op_assign,
    .op_drop        = __x86_64_proxy_bool_op_drop,
    .op_copy        = __x86_64_proxy_bool_op_copy,
    .op_cast        = __x86_64_proxy_bool_op_cast,
    .op_repr        = __x86_64_proxy_bool_op_repr,
    .op_type        = __x86_64_proxy_bool_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_BYTE = {
    .op_plus        = __x86_64_proxy_byte_op_plus,
    .op_minus       = __x86_64_proxy_byte_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_byte_op_bit_not,
    .op_inc         = __x86_64_proxy_byte_op_inc,
    .op_dec         = __x86_64_proxy_byte_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_byte_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_byte_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_byte_op_bit_and,
    .op_eq          = __x86_64_proxy_byte_op_eq,
    .op_neq         = __x86_64_proxy_byte_op_neq,
    .op_less        = __x86_64_proxy_byte_op_less,
    .op_less_eq     = __x86_64_proxy_byte_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_byte_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_byte_op_bit_shr,
    .op_add         = __x86_64_proxy_byte_op_add,
    .op_sub         = __x86_64_proxy_byte_op_sub,
    .op_mul         = __x86_64_proxy_byte_op_mul,
    .op_div         = __x86_64_proxy_byte_op_div,
    .op_rem         = __x86_64_proxy_byte_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_byte_op_assign,
    .op_drop        = __x86_64_proxy_byte_op_drop,
    .op_copy        = __x86_64_proxy_byte_op_copy,
    .op_cast        = __x86_64_proxy_byte_op_cast,
    .op_repr        = __x86_64_proxy_byte_op_repr,
    .op_type        = __x86_64_proxy_byte_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_CALLABLE = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_default_op_eq,
    .op_neq         = __x86_64_proxy_default_op_neq,
    .op_less        = __x86_64_proxy_default_op_less,
    .op_less_eq     = __x86_64_proxy_default_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_default_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_callable_op_call,
    .op_assign      = __x86_64_proxy_callable_op_assign,
    .op_drop        = __x86_64_proxy_callable_op_drop,
    .op_copy        = __x86_64_proxy_callable_op_copy,
    .op_cast        = __x86_64_proxy_default_op_cast,
    .op_repr        = __x86_64_proxy_callable_op_repr,
    .op_type        = __x86_64_proxy_callable_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_CHAR = {
    .op_plus        = __x86_64_proxy_char_op_plus,
    .op_minus       = __x86_64_proxy_char_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_char_op_bit_not,
    .op_inc         = __x86_64_proxy_char_op_inc,
    .op_dec         = __x86_64_proxy_char_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_char_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_char_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_char_op_bit_and,
    .op_eq          = __x86_64_proxy_char_op_eq,
    .op_neq         = __x86_64_proxy_char_op_neq,
    .op_less        = __x86_64_proxy_char_op_less,
    .op_less_eq     = __x86_64_proxy_char_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_char_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_char_op_bit_shr,
    .op_add         = __x86_64_proxy_char_op_add,
    .op_sub         = __x86_64_proxy_char_op_sub,
    .op_mul         = __x86_64_proxy_char_op_mul,
    .op_div         = __x86_64_proxy_char_op_div,
    .op_rem         = __x86_64_proxy_char_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_char_op_assign,
    .op_drop        = __x86_64_proxy_char_op_drop,
    .op_copy        = __x86_64_proxy_char_op_copy,
    .op_cast        = __x86_64_proxy_char_op_cast,
    .op_repr        = __x86_64_proxy_char_op_repr,
    .op_type        = __x86_64_proxy_char_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_ERROR = {
    .op_plus        = __x86_64_proxy_error_op_plus,
    .op_minus       = __x86_64_proxy_error_op_minus,
    .op_not         = __x86_64_proxy_error_op_not,
    .op_bit_not     = __x86_64_proxy_error_op_bit_not,
    .op_inc         = __x86_64_proxy_error_op_inc,
    .op_dec         = __x86_64_proxy_error_op_dec,
    .op_or          = __x86_64_proxy_error_op_or,
    .op_and         = __x86_64_proxy_error_op_and,
    .op_bit_or      = __x86_64_proxy_error_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_error_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_error_op_bit_and,
    .op_eq          = __x86_64_proxy_error_op_eq,
    .op_neq         = __x86_64_proxy_error_op_neq,
    .op_less        = __x86_64_proxy_error_op_less,
    .op_less_eq     = __x86_64_proxy_error_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_error_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_error_op_bit_shr,
    .op_add         = __x86_64_proxy_error_op_add,
    .op_sub         = __x86_64_proxy_error_op_sub,
    .op_mul         = __x86_64_proxy_error_op_mul,
    .op_div         = __x86_64_proxy_error_op_div,
    .op_rem         = __x86_64_proxy_error_op_rem,
    .op_index       = __x86_64_proxy_error_op_index,
    .op_index_v     = __x86_64_proxy_error_op_index_v,
    .op_index_ref   = __x86_64_proxy_error_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_error_op_index_ref_v,
    .op_member      = __x86_64_proxy_error_op_member,
    .op_member_ref  = __x86_64_proxy_error_op_member_ref,
    .op_deref       = __x86_64_proxy_error_op_deref,
    .op_call        = __x86_64_proxy_error_op_call,
    .op_assign      = __x86_64_proxy_error_op_assign,
    .op_drop        = __x86_64_proxy_error_op_drop,
    .op_copy        = __x86_64_proxy_error_op_copy,
    .op_cast        = __x86_64_proxy_error_op_cast,
    .op_repr        = __x86_64_proxy_error_op_repr,
    .op_type        = __x86_64_proxy_error_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_INT = {
    .op_plus        = __x86_64_proxy_int_op_plus,
    .op_minus       = __x86_64_proxy_int_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_int_op_bit_not,
    .op_inc         = __x86_64_proxy_int_op_inc,
    .op_dec         = __x86_64_proxy_int_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_int_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_int_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_int_op_bit_and,
    .op_eq          = __x86_64_proxy_int_op_eq,
    .op_neq         = __x86_64_proxy_int_op_neq,
    .op_less        = __x86_64_proxy_int_op_less,
    .op_less_eq     = __x86_64_proxy_int_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_int_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_int_op_bit_shr,
    .op_add         = __x86_64_proxy_int_op_add,
    .op_sub         = __x86_64_proxy_int_op_sub,
    .op_mul         = __x86_64_proxy_int_op_mul,
    .op_div         = __x86_64_proxy_int_op_div,
    .op_rem         = __x86_64_proxy_int_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_int_op_assign,
    .op_drop        = __x86_64_proxy_int_op_drop,
    .op_copy        = __x86_64_proxy_int_op_copy,
    .op_cast        = __x86_64_proxy_int_op_cast,
    .op_repr        = __x86_64_proxy_int_op_repr,
    .op_type        = __x86_64_proxy_int_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_LONG = {
    .op_plus        = __x86_64_proxy_long_op_plus,
    .op_minus       = __x86_64_proxy_long_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_long_op_bit_not,
    .op_inc         = __x86_64_proxy_long_op_inc,
    .op_dec         = __x86_64_proxy_long_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_long_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_long_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_long_op_bit_and,
    .op_eq          = __x86_64_proxy_long_op_eq,
    .op_neq         = __x86_64_proxy_long_op_neq,
    .op_less        = __x86_64_proxy_long_op_less,
    .op_less_eq     = __x86_64_proxy_long_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_long_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_long_op_bit_shr,
    .op_add         = __x86_64_proxy_long_op_add,
    .op_sub         = __x86_64_proxy_long_op_sub,
    .op_mul         = __x86_64_proxy_long_op_mul,
    .op_div         = __x86_64_proxy_long_op_div,
    .op_rem         = __x86_64_proxy_long_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_long_op_assign,
    .op_drop        = __x86_64_proxy_long_op_drop,
    .op_copy        = __x86_64_proxy_long_op_copy,
    .op_cast        = __x86_64_proxy_long_op_cast,
    .op_repr        = __x86_64_proxy_long_op_repr,
    .op_type        = __x86_64_proxy_long_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_OBJECT = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_default_op_eq,
    .op_neq         = __x86_64_proxy_default_op_neq,
    .op_less        = __x86_64_proxy_default_op_less,
    .op_less_eq     = __x86_64_proxy_default_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_default_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_object_op_index,
    .op_index_v     = __x86_64_proxy_object_op_index_v,
    .op_index_ref   = __x86_64_proxy_object_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_object_op_index_ref_v,
    .op_member      = __x86_64_proxy_object_op_member,
    .op_member_ref  = __x86_64_proxy_object_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_object_op_assign,
    .op_drop        = __x86_64_proxy_object_op_drop,
    .op_copy        = __x86_64_proxy_object_op_copy,
    .op_cast        = __x86_64_proxy_default_op_cast,
    .op_repr        = __x86_64_proxy_object_op_repr,
    .op_type        = __x86_64_proxy_object_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_STRING = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_string_op_eq,
    .op_neq         = __x86_64_proxy_string_op_neq,
    .op_less        = __x86_64_proxy_string_op_less,
    .op_less_eq     = __x86_64_proxy_string_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_string_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_string_op_index,
    .op_index_v     = __x86_64_proxy_string_op_index_v,
    .op_index_ref   = __x86_64_proxy_string_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_string_op_index_ref_v,
    .op_member      = __x86_64_proxy_string_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_string_op_assign,
    .op_drop        = __x86_64_proxy_string_op_drop,
    .op_copy        = __x86_64_proxy_string_op_copy,
    .op_cast        = __x86_64_proxy_default_op_cast,
    .op_repr        = __x86_64_proxy_string_op_repr,
    .op_type        = __x86_64_proxy_string_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_STRING_ELEM_REF = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_default_op_eq,
    .op_neq         = __x86_64_proxy_default_op_neq,
    .op_less        = __x86_64_proxy_default_op_less,
    .op_less_eq     = __x86_64_proxy_default_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_default_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_string_elem_ref_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_string_elem_ref_op_assign,
    .op_drop        = __x86_64_proxy_string_elem_ref_op_drop,
    .op_copy        = __x86_64_proxy_string_elem_ref_op_copy,
    .op_cast        = __x86_64_proxy_default_op_cast,
    .op_repr        = __x86_64_proxy_string_elem_ref_op_repr,
    .op_type        = __x86_64_proxy_string_elem_ref_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_UINT = {
    .op_plus        = __x86_64_proxy_uint_op_plus,
    .op_minus       = __x86_64_proxy_uint_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_uint_op_bit_not,
    .op_inc         = __x86_64_proxy_uint_op_inc,
    .op_dec         = __x86_64_proxy_uint_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_uint_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_uint_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_uint_op_bit_and,
    .op_eq          = __x86_64_proxy_uint_op_eq,
    .op_neq         = __x86_64_proxy_uint_op_neq,
    .op_less        = __x86_64_proxy_uint_op_less,
    .op_less_eq     = __x86_64_proxy_uint_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_uint_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_uint_op_bit_shr,
    .op_add         = __x86_64_proxy_uint_op_add,
    .op_sub         = __x86_64_proxy_uint_op_sub,
    .op_mul         = __x86_64_proxy_uint_op_mul,
    .op_div         = __x86_64_proxy_uint_op_div,
    .op_rem         = __x86_64_proxy_uint_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_uint_op_assign,
    .op_drop        = __x86_64_proxy_uint_op_drop,
    .op_copy        = __x86_64_proxy_uint_op_copy,
    .op_cast        = __x86_64_proxy_uint_op_cast,
    .op_repr        = __x86_64_proxy_uint_op_repr,
    .op_type        = __x86_64_proxy_uint_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_ULONG = {
    .op_plus        = __x86_64_proxy_ulong_op_plus,
    .op_minus       = __x86_64_proxy_ulong_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_ulong_op_bit_not,
    .op_inc         = __x86_64_proxy_ulong_op_inc,
    .op_dec         = __x86_64_proxy_ulong_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_ulong_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_ulong_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_ulong_op_bit_and,
    .op_eq          = __x86_64_proxy_ulong_op_eq,
    .op_neq         = __x86_64_proxy_ulong_op_neq,
    .op_less        = __x86_64_proxy_ulong_op_less,
    .op_less_eq     = __x86_64_proxy_ulong_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_ulong_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_ulong_op_bit_shr,
    .op_add         = __x86_64_proxy_ulong_op_add,
    .op_sub         = __x86_64_proxy_ulong_op_sub,
    .op_mul         = __x86_64_proxy_ulong_op_mul,
    .op_div         = __x86_64_proxy_ulong_op_div,
    .op_rem         = __x86_64_proxy_ulong_op_rem,
    .op_index       = __x86_64_proxy_default_op_index,
    .op_index_v     = __x86_64_proxy_default_op_index_v,
    .op_index_ref   = __x86_64_proxy_default_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_default_op_index_ref_v,
    .op_member      = __x86_64_proxy_default_op_member,
    .op_member_ref  = __x86_64_proxy_default_op_member_ref,
    .op_deref       = __x86_64_proxy_default_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_ulong_op_assign,
    .op_drop        = __x86_64_proxy_ulong_op_drop,
    .op_copy        = __x86_64_proxy_ulong_op_copy,
    .op_cast        = __x86_64_proxy_ulong_op_cast,
    .op_repr        = __x86_64_proxy_ulong_op_repr,
    .op_type        = __x86_64_proxy_ulong_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_VALUE_REF = {
    .op_plus        = __x86_64_proxy_default_op_plus,
    .op_minus       = __x86_64_proxy_default_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_default_op_bit_not,
    .op_inc         = __x86_64_proxy_default_op_inc,
    .op_dec         = __x86_64_proxy_default_op_dec,
    .op_or          = __x86_64_proxy_default_op_or,
    .op_and         = __x86_64_proxy_default_op_and,
    .op_bit_or      = __x86_64_proxy_default_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_default_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_default_op_bit_and,
    .op_eq          = __x86_64_proxy_default_op_eq,
    .op_neq         = __x86_64_proxy_default_op_neq,
    .op_less        = __x86_64_proxy_default_op_less,
    .op_less_eq     = __x86_64_proxy_default_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_default_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_default_op_bit_shr,
    .op_add         = __x86_64_proxy_default_op_add,
    .op_sub         = __x86_64_proxy_default_op_sub,
    .op_mul         = __x86_64_proxy_default_op_mul,
    .op_div         = __x86_64_proxy_default_op_div,
    .op_rem         = __x86_64_proxy_default_op_rem,
    .op_index       = __x86_64_proxy_value_ref_op_index,
    .op_index_v     = __x86_64_proxy_value_ref_op_index_v,
    .op_index_ref   = __x86_64_proxy_value_ref_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_value_ref_op_index_ref_v,
    .op_member      = __x86_64_proxy_value_ref_op_member,
    .op_member_ref  = __x86_64_proxy_value_ref_op_member_ref,
    .op_deref       = __x86_64_proxy_value_ref_op_deref,
    .op_call        = __x86_64_proxy_default_op_call,
    .op_assign      = __x86_64_proxy_value_ref_op_assign,
    .op_drop        = __x86_64_proxy_value_ref_op_drop,
    .op_copy        = __x86_64_proxy_value_ref_op_copy,
    .op_cast        = __x86_64_proxy_default_op_cast,
    .op_repr        = __x86_64_proxy_value_ref_op_repr,
    .op_type        = __x86_64_proxy_value_ref_op_type,
};

static const x86_64_op_tbl X86_64_OP_TBL_VOID = {
    .op_plus        = __x86_64_proxy_void_op_plus,
    .op_minus       = __x86_64_proxy_void_op_minus,
    .op_not         = __x86_64_proxy_default_op_not,
    .op_bit_not     = __x86_64_proxy_void_op_bit_not,
    .op_inc         = __x86_64_proxy_void_op_inc,
    .op_dec         = __x86_64_proxy_void_op_dec,
    .op_or          = __x86_64_proxy_void_op_or,
    .op_and         = __x86_64_proxy_void_op_and,
    .op_bit_or      = __x86_64_proxy_void_op_bit_or,
    .op_bit_xor     = __x86_64_proxy_void_op_bit_xor,
    .op_bit_and     = __x86_64_proxy_void_op_bit_and,
    .op_eq          = __x86_64_proxy_void_op_eq,
    .op_neq         = __x86_64_proxy_void_op_neq,
    .op_less        = __x86_64_proxy_void_op_less,
    .op_less_eq     = __x86_64_proxy_void_op_less_eq,
    .op_bit_shl     = __x86_64_proxy_void_op_bit_shl,
    .op_bit_shr     = __x86_64_proxy_void_op_bit_shr,
    .op_add         = __x86_64_proxy_void_op_add,
    .op_sub         = __x86_64_proxy_void_op_sub,
    .op_mul         = __x86_64_proxy_void_op_mul,
    .op_div         = __x86_64_proxy_void_op_div,
    .op_rem         = __x86_64_proxy_void_op_rem,
    .op_index       = __x86_64_proxy_void_op_index,
    .op_index_v     = __x86_64_proxy_void_op_index_v,
    .op_index_ref   = __x86_64_proxy_void_op_index_ref,
    .op_index_ref_v = __x86_64_proxy_void_op_index_ref_v,
    .op_member      = __x86_64_proxy_void_op_member,
    .op_member_ref  = __x86_64_proxy_void_op_member_ref,
    .op_deref       = __x86_64_proxy_void_op_deref,
    .op_call        = __x86_64_proxy_void_op_call,
    .op_assign      = __x86_64_proxy_void_op_assign,
    .op_drop        = __x86_64_proxy_void_op_drop,
    .op_copy        = __x86_64_proxy_void_op_copy,
    .op_cast        = __x86_64_proxy_void_op_cast,
    .op_repr        = __x86_64_proxy_void_op_repr,
    .op_type        = __x86_64_proxy_void_op_type,
};

static const x86_64_op_tbl *X86_64_OP_TBL_ARR[] = {
    [X86_64_TYPE_ARRAY]           = &X86_64_OP_TBL_ARRAY,
    [X86_64_TYPE_BOOL]            = &X86_64_OP_TBL_BOOL,
    [X86_64_TYPE_BYTE]            = &X86_64_OP_TBL_BYTE,
    [X86_64_TYPE_CALLABLE]        = &X86_64_OP_TBL_CALLABLE,
    [X86_64_TYPE_CHAR]            = &X86_64_OP_TBL_CHAR,
    [X86_64_TYPE_ERROR]           = &X86_64_OP_TBL_ERROR,
    [X86_64_TYPE_INT]             = &X86_64_OP_TBL_INT,
    [X86_64_TYPE_LONG]            = &X86_64_OP_TBL_LONG,
    [X86_64_TYPE_OBJECT]          = &X86_64_OP_TBL_OBJECT,
    [X86_64_TYPE_STRING]          = &X86_64_OP_TBL_STRING,
    [X86_64_TYPE_STRING_ELEM_REF] = &X86_64_OP_TBL_STRING_ELEM_REF,
    [X86_64_TYPE_UINT]            = &X86_64_OP_TBL_UINT,
    [X86_64_TYPE_ULONG]           = &X86_64_OP_TBL_ULONG,
    [X86_64_TYPE_VALUE_REF]       = &X86_64_OP_TBL_VALUE_REF,
    [X86_64_TYPE_VOID]            = &X86_64_OP_TBL_VOID,
};

const x86_64_registry X86_64_REGISTRY = {
    .op_tbl_arr = X86_64_OP_TBL_ARR,
};
