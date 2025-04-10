#include "node_subroutine.h"

#include "compiler/hir/node_basic.h"
#include "util/log.h"
#include "util/macro.h"

hir_subroutine_body *hir_subroutine_body_new_block(list_hir_var   *vars,
                                                   hir_stmt_block *block) {
  hir_subroutine_body *self = MALLOC(hir_subroutine_body);
  self->kind                = HIR_SUBROUTINE_BODY_BLOCK;
  self->body.block.vars     = vars;
  self->body.block.block    = block;
  return self;
}

hir_subroutine_body *hir_subroutine_body_new_import(hir_lit *entry,
                                                    hir_lit *lib) {
  hir_subroutine_body *self = MALLOC(hir_subroutine_body);
  self->kind                = HIR_SUBROUTINE_BODY_IMPORT;
  self->body.import.entry   = entry;
  self->body.import.lib     = lib;
  return self;
}

void hir_subroutine_body_free(hir_subroutine_body *self) {
  if (self) {
    switch (self->kind) {
      case HIR_SUBROUTINE_BODY_IMPORT:
        hir_lit_free(self->body.import.entry);
        hir_lit_free(self->body.import.lib);
        break;
      case HIR_SUBROUTINE_BODY_BLOCK:
        hir_stmt_block_free(self->body.block.block);
        list_hir_var_free(self->body.block.vars);
        break;
      default:
        error("unexpected body kind %d %p", self->kind, self);
        break;
    }
    free(self);
  }
}

hir_subroutine *hir_subroutine_new(span *span, hir_id *id,
                                   list_hir_param      *params,
                                   hir_type_base       *ret_type,
                                   hir_subroutine_spec  spec,
                                   hir_subroutine_body *body) {
  hir_subroutine *self = MALLOC(hir_subroutine);
  hir_base_init(&self->base, span, HIR_NODE_SUBROUTINE);
  self->state    = HIR_STATE_INITIAL;
  self->id_hir   = id;
  self->params   = params;
  self->type_ret = ret_type;
  self->spec     = spec;
  self->body     = body;
  return self;
}

hir_subroutine *hir_subroutine_new_typed(span *span, hir_id *id,
                                         list_hir_param      *params,
                                         type_entry          *type_ref,
                                         hir_subroutine_spec  spec,
                                         hir_subroutine_body *body) {
  hir_subroutine *self = MALLOC(hir_subroutine);
  hir_base_init(&self->base, span, HIR_NODE_SUBROUTINE);
  self->state    = HIR_STATE_INITIAL | HIR_STATE_BIND_TYPE;
  self->id_hir   = id;
  self->params   = params;
  self->type_ref = type_ref;
  self->spec     = spec;
  self->body     = body;
  return self;
}

void hir_subroutine_free(hir_subroutine *self) {
  if (self) {
    if (!(self->state & HIR_STATE_BIND_SYMBOL)) {
      hir_id_free(self->id_hir);
    }
    list_hir_param_free(self->params);
    if (!(self->state & HIR_STATE_BIND_TYPE)) {
      hir_type_free(self->type_ret);
    }
    hir_subroutine_body_free(self->body);
    hir_base_deinit(&self->base);
    free(self);
  }
}
