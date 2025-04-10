#pragma once

#include "x86_64_core/value.h"

typedef struct __attribute__((packed)) x86_64_data_array_struct {
  uint64_t     ref_cnt;
  uint64_t     length;
  x86_64_value elements[0];
} x86_64_data_array;
