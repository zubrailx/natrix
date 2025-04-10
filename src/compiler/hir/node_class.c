#include "node_class.h"
#include "compiler/hir/node.h"
#include "util/macro.h"

hir_method *hir_method_new(span *span, hir_method_modifier_enum modifier,
                           hir_subroutine *subroutine) {
  hir_method *self = MALLOC(hir_method);
  hir_base_init(&self->base, span, HIR_NODE_METHOD);
  self->modifier   = modifier;
  self->subroutine = subroutine;
  return self;
}

void hir_method_free(hir_method *self) {
  if (self) {
    hir_subroutine_free(self->subroutine);
    hir_base_deinit(&self->base);
    free(self);
  }
}

hir_class *hir_class_new(span *span, hir_id *id, list_hir_id *typenames,
                         list_hir_type *parents, list_hir_var *fields,
                         list_hir_method *methods) {
  hir_class *self = MALLOC(hir_class);
  hir_base_init(&self->base, span, HIR_NODE_CLASS);
  self->state     = HIR_STATE_INITIAL;
  self->id        = id;
  self->typenames = typenames;
  self->parents   = parents;
  self->fields    = fields;
  self->methods   = methods;
  return self;
}

hir_class *hir_class_new_typed(span *span, type_entry *type_ref,
                               list_hir_var *fields, list_hir_method *methods) {
  hir_class *self = MALLOC(hir_class);
  hir_base_init(&self->base, span, HIR_NODE_CLASS);
  self->state    = HIR_STATE_INITIAL | HIR_STATE_BIND_TYPE;
  self->type_ref = type_ref;
  self->fields   = fields;
  self->methods  = methods;
  return self;
}

void hir_class_free(hir_class *self) {
  if (self) {
    if (!(self->state & HIR_STATE_BIND_TYPE)) {
      hir_id_free(self->id);
      list_hir_id_free(self->typenames);
      list_hir_type_free(self->parents);
    }
    list_hir_var_free(self->fields);
    list_hir_method_free(self->methods);
    hir_base_deinit(&self->base);
    free(self);
  }
}
