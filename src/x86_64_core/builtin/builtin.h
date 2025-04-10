#pragma once

#include "x86_64_core/value.h"
#include "x86_64_core/value/object.h"

// io
void __x86_64_flush();

// make
void __x86_64_make_void(x86_64_value *out);
void __x86_64_make_bool(x86_64_value *out, uint8_t value);
void __x86_64_make_byte(x86_64_value *out, uint8_t value);
void __x86_64_make_int(x86_64_value *out, int32_t value);
void __x86_64_make_uint(x86_64_value *out, uint32_t value);
void __x86_64_make_long(x86_64_value *out, int64_t value);
void __x86_64_make_ulong(x86_64_value *out, uint64_t value);
void __x86_64_make_char(x86_64_value *out, uint8_t value);
void __x86_64_make_string(x86_64_value *out, const uint8_t *value);
void __x86_64_make_string_move(x86_64_value *out, uint8_t *value);
void __x86_64_make_callable(x86_64_value *out, x86_64_func *func);
// args are of type x86_64_value *
void __x86_64_make_array(x86_64_value *out, ...);
void __x86_64_make_object(x86_64_value                     *out,
                          const x86_64_data_object_symbols *symbols);
// move defaults into object, for faster default setup
void __x86_64_make_object_setup(x86_64_value                     *out,
                                const x86_64_data_object_symbols *symbols,
                                x86_64_value                     *defaults);
void __x86_64_make_error(x86_64_value *out, x86_64_value *value);

// print
void __x86_64_print(x86_64_value *self);

// unwrap
void     __x86_64_unwrap_void(x86_64_value *out);
uint8_t  __x86_64_unwrap_bool(x86_64_value *out);
uint8_t  __x86_64_unwrap_byte(x86_64_value *out);
int32_t  __x86_64_unwrap_int(x86_64_value *out);
uint32_t __x86_64_unwrap_uint(x86_64_value *out);
int64_t  __x86_64_unwrap_long(x86_64_value *out);
uint64_t __x86_64_unwrap_ulong(x86_64_value *out);
uint8_t  __x86_64_unwrap_char(x86_64_value *out);
uint8_t *__x86_64_unwrap_string(x86_64_value *out);
