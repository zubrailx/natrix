#pragma once

#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/exception/list.h"

typedef struct cg_x86_64_emit_ctx_struct {
  list_exception *exceptions;
} cg_x86_64_emit_ctx;

char *cg_x86_64_emit_gas(cg_x86_64_emit_ctx *ctx, const cg_x86_64 *code);
