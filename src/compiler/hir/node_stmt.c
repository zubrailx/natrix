#include "node_stmt.h"
#include "util/log.h"
#include "util/macro.h"

void hir_stmt_if_free(hir_stmt_if *self);
void hir_stmt_block_free(hir_stmt_block *self);
void hir_stmt_while_free(hir_stmt_while *self);
void hir_stmt_do_free(hir_stmt_do *self);
void hir_stmt_break_free(hir_stmt_break *self);
void hir_stmt_expr_free(hir_stmt_expr *self);

void hir_stmt_base_init(hir_stmt_base *self, span *span, hir_stmt_enum kind) {
  hir_base_init(&self->base, span, HIR_NODE_STMT);
  self->kind = kind;
}

void hir_stmt_base_deinit(hir_stmt_base *self) { hir_base_deinit(&self->base); }

void hir_stmt_free(hir_stmt_base *generic) {
  if (generic) {
    switch (generic->kind) {
      case HIR_STMT_IF:
        return hir_stmt_if_free((hir_stmt_if *)generic);
      case HIR_STMT_BLOCK:
        return hir_stmt_block_free((hir_stmt_block *)generic);
      case HIR_STMT_WHILE:
        return hir_stmt_while_free((hir_stmt_while *)generic);
      case HIR_STMT_DO:
        return hir_stmt_do_free((hir_stmt_do *)generic);
      case HIR_STMT_BREAK:
        return hir_stmt_break_free((hir_stmt_break *)generic);
      case HIR_STMT_EXPR:
        return hir_stmt_expr_free((hir_stmt_expr *)generic);
      case HIR_STMT_RETURN:
        return hir_stmt_return_free((hir_stmt_return *)generic);
    }
    error("unknown stmt kind to free %d %p", generic->kind, generic);
  }
}

hir_stmt_if *hir_stmt_if_new(span *span, hir_expr_base *cond, hir_stmt_base *je,
                             hir_stmt_base *jz) {
  hir_stmt_if *self = MALLOC(hir_stmt_if);
  hir_stmt_base_init(&self->base, span, HIR_STMT_IF);
  self->cond = cond;
  self->je   = je;
  self->jz   = jz;
  return self;
}

void hir_stmt_if_free(hir_stmt_if *self) {
  if (self) {
    hir_expr_free(self->cond);
    hir_stmt_free(self->je);
    hir_stmt_free(self->jz);
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}

hir_stmt_block *hir_stmt_block_new(span *span, list_hir_stmt *stmts) {
  hir_stmt_block *self = MALLOC(hir_stmt_block);
  hir_stmt_base_init(&self->base, span, HIR_STMT_BLOCK);
  self->stmts = stmts;
  return self;
}

void hir_stmt_block_free(hir_stmt_block *self) {
  if (self) {
    list_hir_stmt_free(self->stmts);
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}

hir_stmt_while *hir_stmt_while_new(span *span, hir_expr_base *cond,
                                   hir_stmt_base *stmt) {
  hir_stmt_while *self = MALLOC(hir_stmt_while);
  hir_stmt_base_init(&self->base, span, HIR_STMT_WHILE);
  self->cond = cond;
  self->stmt = stmt;
  return self;
}

void hir_stmt_while_free(hir_stmt_while *self) {
  if (self) {
    hir_expr_free(self->cond);
    hir_stmt_free(self->stmt);
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}

hir_stmt_do *hir_stmt_do_new(span *span, int positive, hir_expr_base *cond,
                             hir_stmt_base *stmt) {
  hir_stmt_do *self = MALLOC(hir_stmt_do);
  hir_stmt_base_init(&self->base, span, HIR_STMT_DO);
  self->positive = positive;
  self->cond     = cond;
  self->stmt     = stmt;
  return self;
}

void hir_stmt_do_free(hir_stmt_do *self) {
  if (self) {
    hir_expr_free(self->cond);
    hir_stmt_free(self->stmt);
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}

hir_stmt_break *hir_stmt_break_new(span *span) {
  hir_stmt_break *self = MALLOC(hir_stmt_break);
  hir_stmt_base_init(&self->base, span, HIR_STMT_BREAK);
  return self;
}

void hir_stmt_break_free(hir_stmt_break *self) {
  if (self) {
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}

hir_stmt_expr *hir_stmt_expr_new(span *span, hir_expr_base *expr) {
  hir_stmt_expr *self = MALLOC(hir_stmt_expr);
  hir_stmt_base_init(&self->base, span, HIR_STMT_EXPR);
  self->expr = expr;
  return self;
}

void hir_stmt_expr_free(hir_stmt_expr *self) {
  if (self) {
    hir_expr_free(self->expr);
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}

hir_stmt_return *hir_stmt_return_new(span *span, hir_expr_base *expr) {
  hir_stmt_return *self = MALLOC(hir_stmt_return);
  hir_stmt_base_init(&self->base, span, HIR_STMT_RETURN);
  self->expr = expr;
  return self;
}

void hir_stmt_return_free(hir_stmt_return *self) {
  if (self) {
    hir_expr_free(self->expr);
    hir_stmt_base_deinit(&self->base);
    free(self);
  }
}
