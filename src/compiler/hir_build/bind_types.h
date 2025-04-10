#pragma once

#include "compiler/exception/list.h"
#include "compiler/hir/hir.h"
#include "compiler/type_table/type_table.h"

typedef struct hir_bind_types_result_struct {
  type_table     *type_table;
  list_exception *exceptions;
} hir_bind_types_result;

hir_bind_types_result hir_bind_types(hir *hir);
