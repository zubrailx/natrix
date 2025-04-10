#pragma once

#include "x86_64_core/value.h"

typedef struct __attribute__((packed)) x86_64_data_callable_struct {
  x86_64_func *func;
} x86_64_data_callable;
