#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) x86_64_data_char_struct {
  uint8_t value;
  uint8_t pad[7];
} x86_64_data_char;
