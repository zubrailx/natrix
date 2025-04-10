#pragma once

#include "x86_64_core/value.h"
#include <stdint.h>

typedef struct __attribute__((packed)) x86_64_data_object_symbol_struct {
  const uint8_t *name;
} x86_64_data_object_symbol;

typedef struct __attribute__((packed)) x86_64_data_object_symbols_struct {
  uint64_t                  count;
  x86_64_data_object_symbol symbols[0];
} x86_64_data_object_symbols;

typedef struct __attribute__((packed)) x86_64_data_object_struct {
  uint64_t                          ref_cnt;
  const x86_64_data_object_symbols *symbols_ref;
  x86_64_value                      members[0];
} x86_64_data_object;
