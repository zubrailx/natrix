#pragma once

#include "compiler/exception/list.h"
#include "compiler/hir/hir.h"
#include "compiler/mir/mir.h"

typedef struct mir_build_result_struct {
  mir            *mir;
  list_exception *exceptions;
} mir_build_result;

mir_build_result mir_build(const hir *hir, const symbol_table *symbol_table,
                           const type_table *type_table, int ignore_errors);
