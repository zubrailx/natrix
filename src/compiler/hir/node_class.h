#pragma once

#include "compiler/hir/node.h"
#include "compiler/hir/node_basic.h"
#include "compiler/hir/node_subroutine.h"
#include "util/list.h"

typedef enum hir_method_modifier_enum {
  HIR_METHOD_MODIFIER_ENUM_EMPTY,
  HIR_METHOD_MODIFIER_ENUM_PUBLIC,
  HIR_METHOD_MODIFIER_ENUM_PRIVATE,
} hir_method_modifier_enum;

typedef struct hir_method_struct {
  hir_base                 base;
  hir_method_modifier_enum modifier;
  hir_subroutine          *subroutine;
} hir_method;

hir_method *hir_method_new(span *span, hir_method_modifier_enum modifier,
                           hir_subroutine *subroutine);
void        hir_method_free(hir_method *self);

static inline void container_delete_hir_method(void *data) {
  hir_method_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_hir_method, hir_method, container_cmp_false,
                           container_new_move, container_delete_hir_method);

typedef struct hir_class_struct {
  hir_base       base;
  hir_state_enum state;
  union {
    struct {
      hir_id        *id;
      list_hir_id   *typenames;
      list_hir_type *parents;
    };
    type_entry *type_ref;
  };
  list_hir_var    *fields;
  list_hir_method *methods;
} hir_class;

hir_class *hir_class_new(span *span, hir_id *id, list_hir_id *typenames,
                         list_hir_type *parents, list_hir_var *fields,
                         list_hir_method *methods);
hir_class *hir_class_new_typed(span *span, type_entry *type_ref,
                               list_hir_var *fields, list_hir_method *methods);
void       hir_class_free(hir_class *self);

static inline void container_delete_hir_class(void *data) {
  hir_class_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_hir_class, hir_class, container_cmp_false,
                           container_new_move, container_delete_hir_class);
