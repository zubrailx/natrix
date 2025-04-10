#pragma once

#include "compiler/ast/ast.h"
#include "compiler/exception/list.h"
#include "compiler/hir/hir.h"

typedef struct hir_lower_ast_ctx_struct {
  hir            *hir_ref;
  list_exception *exceptions;
  const ast      *ast_cur_ref;
} hir_lower_ast_ctx;

typedef struct hir_lower_ast_result_struct {
  hir            *hir;
  list_exception *exceptions;
} hir_lower_ast_result;

hir_lower_ast_result hir_lower_ast(const list_ast *asts);
