#pragma once

#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/exception/list.h"

typedef enum cg_x86_64_emit_format_enum {
  CG_X86_64_EMIT_FORMAT_GAS,
} cg_x86_64_emit_format;

typedef struct cg_x86_64_emit_result_struct {
  char           *asm_str;
  list_exception *exceptions;
} cg_x86_64_emit_result;

cg_x86_64_emit_result cg_x86_64_emit(const cg_x86_64      *code,
                                     cg_x86_64_emit_format format);
