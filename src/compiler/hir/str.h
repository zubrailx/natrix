#pragma once

#include "compiler/hir/hir.h"
#include "compiler/hir/node_expr.h"
#include "compiler/hir/node_stmt.h"
#include "compiler/hir/type.h"

char *hir_type_str(const hir_type_base *generic);

const char *hir_expr_unary_enum_str(hir_expr_unary_enum type);
const char *hir_expr_binary_enum_str(hir_expr_binary_enum type);
const char *hir_node_enum_str(hir_node_enum kind);
const char *hir_stmt_enum_str(hir_stmt_enum kind);
const char *hir_expr_enum_str(hir_expr_enum kind);
const char *hir_expr_builtin_enum_str(hir_expr_builtin_enum kind);
char       *hir_lit_value_str(const hir_lit *lit);
const char *hir_method_modifier_enum_str(hir_method_modifier_enum kind);
const char *hir_state_enum_str(hir_state_enum state);
const char *hir_subroutine_spec_str(hir_subroutine_spec spec);

const char *hir_type_enum_str(hir_type_enum type);
char       *hir_tree_str(const hir *hir);
const char *hir_type_base_str(hir_type_base *type);
