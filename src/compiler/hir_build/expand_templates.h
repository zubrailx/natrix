#pragma once

#include "compiler/exception/list.h"
#include "compiler/hir/hir.h"

typedef struct hir_expand_templates_result_struct {
  list_exception *exceptions;
} hir_expand_templates_result;

// modifies type_table, creates new HIR with expanded templates
hir_expand_templates_result hir_expand_templates(hir        *hir,
                                                 type_table *type_table);
