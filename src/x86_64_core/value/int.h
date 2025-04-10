#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) x86_64_data_int_struct {
  int32_t value;
  uint8_t pad[4];
} x86_64_data_int;
