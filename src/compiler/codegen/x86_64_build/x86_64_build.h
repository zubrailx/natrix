#pragma once

#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/exception/list.h"
#include "compiler/mir/mir.h"

typedef struct cg_x86_64_build_result_struct {
  cg_x86_64      *code;
  list_exception *exceptions;
} cg_x86_64_build_result;

cg_x86_64_build_result cg_x86_64_build(const mir *mir, int ignore_errors);
