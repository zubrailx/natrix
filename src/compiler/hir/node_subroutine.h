#pragma once

#include "compiler/hir/node.h"
#include "compiler/hir/node_basic.h"
#include "compiler/hir/node_stmt.h"

typedef enum hir_subroutine_body_enum {
  HIR_SUBROUTINE_BODY_IMPORT = 1,
  HIR_SUBROUTINE_BODY_BLOCK,
} hir_subroutine_body_enum;

typedef enum hir_subroutine_spec_enum {
  HIR_SUBROUTINE_SPEC_EMPTY,
  HIR_SUBROUTINE_SPEC_EXTERN = 1 << 0,
} hir_subroutine_spec;

typedef struct hir_subroutine_body {
  hir_subroutine_body_enum kind;
  union {
    struct {
      list_hir_var   *vars;
      hir_stmt_block *block;
    } block;
    struct {
      hir_lit *entry;
      hir_lit *lib;
    } import;
  } body;
} hir_subroutine_body;

typedef struct hir_subroutine {
  hir_base       base;
  hir_state_enum state;
  union {
    hir_id       *id_hir;
    symbol_entry *id_ref;
  };
  list_hir_param *params;
  union {
    hir_type_base *type_ret;
    type_entry    *type_ref;
  };
  hir_subroutine_spec  spec;
  hir_subroutine_body *body;
} hir_subroutine;

hir_subroutine_body *hir_subroutine_body_new_block(list_hir_var   *vars,
                                                   hir_stmt_block *block);
hir_subroutine_body *hir_subroutine_body_new_import(hir_lit *entry,
                                                    hir_lit *lib);
void                 hir_subroutine_body_free(hir_subroutine_body *self);

hir_subroutine *hir_subroutine_new(span *span, hir_id *id,
                                   list_hir_param      *params,
                                   hir_type_base       *ret_type,
                                   hir_subroutine_spec  spec,
                                   hir_subroutine_body *body);

hir_subroutine *hir_subroutine_new_typed(span *span, hir_id *id,
                                         list_hir_param      *params,
                                         type_entry          *type_ref,
                                         hir_subroutine_spec  spec,
                                         hir_subroutine_body *body);

void hir_subroutine_free(hir_subroutine *self);

static inline void container_delete_hir_subroutine(void *data) {
  hir_subroutine_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_hir_subroutine, hir_subroutine,
                           container_cmp_false, container_new_move,
                           container_delete_hir_subroutine);
