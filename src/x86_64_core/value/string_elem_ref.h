#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) x86_64_data_string_elem_ref_struct {
  uint8_t *value; // element in string
} x86_64_data_string_elem_ref;
