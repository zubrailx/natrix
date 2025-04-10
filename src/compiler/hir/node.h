#pragma once

#include "compiler/span/span.h"

typedef enum hir_state_enum {
  HIR_STATE_INITIAL              = 0,
  HIR_STATE_BIND_TYPE            = 1 << 0,
  HIR_STATE_BIND_SYMBOL          = 1 << 1,
  HIR_STATE_BIND_SYMBOL_AND_TYPE = HIR_STATE_BIND_SYMBOL | HIR_STATE_BIND_TYPE,
} hir_state_enum;

typedef enum hir_node_enum {
  HIR_NODE_ID = 1,
  HIR_NODE_LIT,
  HIR_NODE_PARAM,
  HIR_NODE_VAR,
  HIR_NODE_SUBROUTINE,
  HIR_NODE_STMT,
  HIR_NODE_EXPR,
  HIR_NODE_CLASS,
  HIR_NODE_METHOD,
} hir_node_enum;

typedef struct hir_base_struct {
  hir_node_enum kind;
  span         *span;
} hir_base;

void hir_base_init(hir_base *self, span *span, hir_node_enum kind);
void hir_base_deinit(hir_base *self);

void hir_node_free(hir_base *generic);
