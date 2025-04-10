#pragma once

#include "x86_64_core/value.h"

x86_64_func __x86_64_proxy_op_call_func_error;

void __x86_64_proxy_op_error_string(x86_64_value *out, const char *op,
                                    const char *err_msg);

void __x86_64_proxy_op_error_undefined(x86_64_value *out, x86_64_value *self,
                                       const char *op);

void __x86_64_proxy_op_error_type_mismatch(x86_64_value *out,
                                           x86_64_value *self, const char *op,
                                           x86_64_type_enum type);

void __x86_64_proxy_op_error_unable_to_cast(x86_64_value    *out,
                                            x86_64_value    *self,
                                            x86_64_type_enum type);

void __x86_64_proxy_op_error_not_number(x86_64_value *out, x86_64_value *self,
                                        const char *op);

void __x86_64_proxy_op_error_no_member(x86_64_value *out, const char *op,
                                       const uint8_t *member);

uint64_t __x86_64_proxy_value_as_index(const x86_64_value *self);
