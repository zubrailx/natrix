#pragma once

#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/codegen/x86_64_build/debug.h"
#include "compiler/exception/list.h"

typedef struct cg_debug_emit_result_struct {
  list_exception *exceptions;
} cg_debug_emit_result;

cg_debug_emit_result cg_debug_emit(cg_x86_64 *code, const cg_debug *debug);
