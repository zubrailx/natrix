#include "node.h"

#include "compiler/hir/node_basic.h"
#include "compiler/hir/node_class.h"
#include "compiler/hir/node_subroutine.h"
#include "util/log.h"

void hir_base_init(hir_base *self, span *span, hir_node_enum kind) {
  self->kind = kind;
  self->span = span;
}

void hir_base_deinit(hir_base *self) { span_free(self->span); }

void hir_node_free(hir_base *generic) {
  if (generic) {
    switch (generic->kind) {
      case HIR_NODE_ID:
        return hir_id_free((hir_id *)generic);
      case HIR_NODE_LIT:
        return hir_lit_free((hir_lit *)generic);
      case HIR_NODE_PARAM:
        return hir_param_free((hir_param *)generic);
      case HIR_NODE_VAR:
        return hir_var_free((hir_var *)generic);
      case HIR_NODE_SUBROUTINE:
        return hir_subroutine_free((hir_subroutine *)generic);
      case HIR_NODE_EXPR:
        return hir_expr_free((hir_expr_base *)generic);
      case HIR_NODE_STMT:
        return hir_stmt_free((hir_stmt_base *)generic);
      case HIR_NODE_CLASS:
        return hir_class_free((hir_class *)generic);
      case HIR_NODE_METHOD:
        return hir_method_free((hir_method *)generic);
        break;
    }
    error("unknown generic node %d %p", generic->kind, generic);
  }
}
