#include "node_expr.h"
#include "compiler/hir/node.h"
#include "util/log.h"
#include "util/macro.h"

void hir_expr_base_init(hir_expr_base *self, span *span, hir_expr_enum kind,
                        hir_type_base *type) {
  hir_base_init(&self->base, span, HIR_NODE_EXPR);
  self->state    = HIR_STATE_INITIAL;
  self->kind     = kind;
  self->type_hir = type;
}

void hir_expr_base_init_typed(hir_expr_base *self, span *span,
                              hir_expr_enum kind, type_entry *type_ref) {
  hir_base_init(&self->base, span, HIR_NODE_EXPR);
  self->state    = HIR_STATE_INITIAL | HIR_STATE_BIND_TYPE;
  self->kind     = kind;
  self->type_ref = type_ref;
}

void hir_expr_base_deinit(hir_expr_base *self) {
  if (!(self->state & HIR_STATE_BIND_TYPE)) {
    hir_type_free(self->type_hir);
  }
  hir_base_deinit(&self->base);
}

void hir_expr_free(hir_expr_base *generic) {
  if (generic) {
    switch (generic->kind) {
      case HIR_EXPR_UNARY:
        return hir_expr_unary_free((hir_expr_unary *)generic);
      case HIR_EXPR_BINARY:
        return hir_expr_binary_free((hir_expr_binary *)generic);
      case HIR_EXPR_LITERAL:
        return hir_expr_lit_free((hir_expr_lit *)generic);
      case HIR_EXPR_IDENTIFIER:
        return hir_expr_id_free((hir_expr_id *)generic);
      case HIR_EXPR_CALL:
        return hir_expr_call_free((hir_expr_call *)generic);
      case HIR_EXPR_INDEX:
        return hir_expr_index_free((hir_expr_index *)generic);
      case HIR_EXPR_BUILTIN:
        return hir_expr_builtin_free((hir_expr_builtin *)generic);
    }
    error("unknown expr type %d, %p", generic->kind, generic);
  }
}

hir_expr_unary *hir_expr_unary_new(span *span, hir_type_base *type,
                                   hir_expr_unary_enum op,
                                   hir_expr_base      *first) {
  hir_expr_unary *self = MALLOC(hir_expr_unary);
  hir_expr_base_init(&self->base, span, HIR_EXPR_UNARY, type);
  self->op    = op;
  self->first = first;
  return self;
}

hir_expr_unary *hir_expr_unary_new_typed(span *span, type_entry *type_ref,
                                         hir_expr_unary_enum op,
                                         hir_expr_base      *first) {
  hir_expr_unary *self = MALLOC(hir_expr_unary);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_UNARY, type_ref);
  self->op    = op;
  self->first = first;
  return self;
}

void hir_expr_unary_free(hir_expr_unary *self) {
  if (self) {
    hir_expr_free(self->first);
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}

hir_expr_binary *hir_expr_binary_new(span *span, hir_type_base *type,
                                     hir_expr_binary_enum op,
                                     hir_expr_base       *first,
                                     hir_expr_base       *second) {
  hir_expr_binary *self = MALLOC(hir_expr_binary);
  hir_expr_base_init(&self->base, span, HIR_EXPR_BINARY, type);
  self->op     = op;
  self->first  = first;
  self->second = second;
  return self;
}

hir_expr_binary *hir_expr_binary_new_typed(span *span, type_entry *type_ref,
                                           hir_expr_binary_enum op,
                                           hir_expr_base       *first,
                                           hir_expr_base       *second) {
  hir_expr_binary *self = MALLOC(hir_expr_binary);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_BINARY, type_ref);
  self->op     = op;
  self->first  = first;
  self->second = second;
  return self;
}

void hir_expr_binary_free(hir_expr_binary *self) {
  if (self) {
    hir_expr_free(self->first);
    hir_expr_free(self->second);
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}

hir_expr_lit *hir_expr_lit_new(span *span, hir_type_base *type, hir_lit *lit) {

  hir_expr_lit *self = MALLOC(hir_expr_lit);
  hir_expr_base_init(&self->base, span, HIR_EXPR_LITERAL, type);
  self->lit = lit;
  return self;
}

hir_expr_lit *hir_expr_lit_new_typed(span *span, type_entry *type_ref,
                                     hir_lit *lit) {
  hir_expr_lit *self = MALLOC(hir_expr_lit);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_LITERAL, type_ref);
  self->lit = lit;
  return self;
}

void hir_expr_lit_free(hir_expr_lit *self) {
  if (self) {
    hir_lit_free(self->lit);
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}

hir_expr_id *hir_expr_id_new(span *span, hir_type_base *type, hir_id *id) {

  hir_expr_id *self = MALLOC(hir_expr_id);
  hir_expr_base_init(&self->base, span, HIR_EXPR_IDENTIFIER, type);
  self->id_hir = id;
  return self;
}

hir_expr_id *hir_expr_id_new_typed(span *span, type_entry *type_ref,
                                   hir_id *id) {
  hir_expr_id *self = MALLOC(hir_expr_id);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_IDENTIFIER, type_ref);
  self->id_hir = id;
  return self;
}

void hir_expr_id_free(hir_expr_id *self) {
  if (self) {
    if (!(self->base.state & HIR_STATE_BIND_SYMBOL)) {
      hir_id_free(self->id_hir);
    }
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}

hir_expr_call *hir_expr_call_new(span *span, hir_type_base *type,
                                 hir_expr_base *callee, list_hir_expr *args) {

  hir_expr_call *self = MALLOC(hir_expr_call);
  hir_expr_base_init(&self->base, span, HIR_EXPR_CALL, type);
  self->callee = callee;
  self->args   = args;
  return self;
}

hir_expr_call *hir_expr_call_new_typed(span *span, type_entry *type_ref,
                                       hir_expr_base *callee,
                                       list_hir_expr *args) {
  hir_expr_call *self = MALLOC(hir_expr_call);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_CALL, type_ref);
  self->callee = callee;
  self->args   = args;
  return self;
}

void hir_expr_call_free(hir_expr_call *self) {
  if (self) {
    hir_expr_free(self->callee);
    list_hir_expr_free(self->args);
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}

hir_expr_index *hir_expr_index_new(span *span, hir_type_base *type,
                                   hir_expr_base *indexed,
                                   list_hir_expr *args) {
  hir_expr_index *self = MALLOC(hir_expr_index);
  hir_expr_base_init(&self->base, span, HIR_EXPR_INDEX, type);
  self->indexed = indexed;
  self->args    = args;
  return self;
}

hir_expr_index *hir_expr_index_new_typed(span *span, type_entry *type_ref,
                                         hir_expr_base *indexed,
                                         list_hir_expr *args) {
  hir_expr_index *self = MALLOC(hir_expr_index);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_INDEX, type_ref);
  self->indexed = indexed;
  self->args    = args;
  return self;
}

void hir_expr_index_free(hir_expr_index *self) {
  if (self) {
    hir_expr_free(self->indexed);
    list_hir_expr_free(self->args);
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}

hir_expr_builtin *hir_expr_builtin_new(span *span, hir_type_base *type,
                                       hir_expr_builtin_enum kind,
                                       list_hir_expr        *args) {
  hir_expr_builtin *self = MALLOC(hir_expr_builtin);
  hir_expr_base_init(&self->base, span, HIR_EXPR_BUILTIN, type);
  self->kind = kind;
  self->args = args;
  return self;
}

hir_expr_builtin *hir_expr_builtin_new_typed(span *span, type_entry *type_ref,
                                             hir_expr_builtin_enum kind,
                                             list_hir_expr        *args) {
  hir_expr_builtin *self = MALLOC(hir_expr_builtin);
  hir_expr_base_init_typed(&self->base, span, HIR_EXPR_BUILTIN, type_ref);
  self->kind = kind;
  self->args = args;
  return self;
}

void hir_expr_builtin_free(hir_expr_builtin *self) {
  if (self) {
    list_hir_expr_free(self->args);
    hir_expr_base_deinit(&self->base);
    free(self);
  }
}
