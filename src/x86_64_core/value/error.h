#pragma once

#include "x86_64_core/value.h"

typedef struct __attribute__((packed)) x86_64_data_error_struct {
  x86_64_value value;
} x86_64_data_error;
