#pragma once

#include "compiler/exception/list.h"
#include "compiler/hir/hir.h"
#include "compiler/mir/mir.h"

typedef struct mir_lower_hir_result_struct {
  mir            *mir;
  list_exception *exceptions;
} mir_lower_hir_result;

mir_lower_hir_result mir_lower_hir(const hir        *hir,
                                   const type_table *type_table);
