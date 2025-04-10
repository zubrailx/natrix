#pragma once

#include "x86_64_core/value.h"

typedef struct x86_64_registry_struct {
  const x86_64_op_tbl **op_tbl_arr;
} x86_64_registry;

extern const x86_64_registry X86_64_REGISTRY;
