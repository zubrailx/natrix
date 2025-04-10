#pragma once

#include "compiler/hir/node.h"
#include "compiler/hir/node_expr.h"

typedef enum hir_stmt_enum {
  HIR_STMT_IF = 1,
  HIR_STMT_BLOCK,
  HIR_STMT_WHILE,
  HIR_STMT_DO,
  HIR_STMT_BREAK,
  HIR_STMT_EXPR,
  HIR_STMT_RETURN,
} hir_stmt_enum;

typedef struct hir_stmt_base_struct {
  hir_base      base;
  hir_stmt_enum kind;
} hir_stmt_base;

void hir_stmt_base_init(hir_stmt_base *self, span *span, hir_stmt_enum kind);
void hir_stmt_base_deinit(hir_stmt_base *self);

void hir_stmt_free(hir_stmt_base *generic);

static inline void container_delete_hir_stmt(void *data) {
  hir_stmt_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_hir_stmt, hir_stmt_base, container_cmp_false,
                           container_new_move, container_delete_hir_stmt);

typedef struct hir_stmt_if_struct {
  hir_stmt_base  base;
  hir_expr_base *cond;
  hir_stmt_base *je;
  hir_stmt_base *jz;
} hir_stmt_if;

hir_stmt_if *hir_stmt_if_new(span *span, hir_expr_base *cond, hir_stmt_base *je,
                             hir_stmt_base *jz);
void         hir_stmt_if_free(hir_stmt_if *self);

typedef struct hir_stmt_block_struct {
  hir_stmt_base  base;
  list_hir_stmt *stmts;
} hir_stmt_block;

hir_stmt_block *hir_stmt_block_new(span *span, list_hir_stmt *stmts);
void            hir_stmt_block_free(hir_stmt_block *self);

typedef struct hir_stmt_while_struct {
  hir_stmt_base  base;
  hir_expr_base *cond;
  hir_stmt_base *stmt;
} hir_stmt_while;

hir_stmt_while *hir_stmt_while_new(span *span, hir_expr_base *cond,
                                   hir_stmt_base *stmt);
void            hir_stmt_while_free(hir_stmt_while *self);

typedef struct hir_stmt_do_struct {
  hir_stmt_base  base;
  int            positive;
  hir_expr_base *cond;
  hir_stmt_base *stmt;
} hir_stmt_do;

hir_stmt_do *hir_stmt_do_new(span *span, int positive, hir_expr_base *cond,
                             hir_stmt_base *stmt);
void         hir_stmt_do_free(hir_stmt_do *self);

typedef struct hir_stmt_break_struct {
  hir_stmt_base base;
} hir_stmt_break;

hir_stmt_break *hir_stmt_break_new(span *span);
void            hir_stmt_break_free(hir_stmt_break *self);

typedef struct hir_stmt_expr_struct {
  hir_stmt_base  base;
  hir_expr_base *expr;
} hir_stmt_expr;

hir_stmt_expr *hir_stmt_expr_new(span *span, hir_expr_base *expr);
void           hir_stmt_expr_free(hir_stmt_expr *self);

typedef struct hir_stmt_return_struct {
  hir_stmt_base  base;
  hir_expr_base *expr;
} hir_stmt_return;

hir_stmt_return *hir_stmt_return_new(span *span, hir_expr_base *expr);
void             hir_stmt_return_free(hir_stmt_return *self);
