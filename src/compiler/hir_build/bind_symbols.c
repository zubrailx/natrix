#include "bind_symbols.h"

#include "compiler/hir_build/exception.h"
#include "util/hashset.h"
#include "util/log.h"
#include "util/macro.h"
#include <string.h>

// hir_scope
static inline int container_cmp_hir_scope_entry(const void *lsv,
                                                const void *rsv) {
  const symbol_entry *l = lsv;
  const symbol_entry *r = rsv;
  return container_cmp_chars(l->name, r->name);
}
static inline uint64_t container_hash_hir_scope_entry(const void *lsv) {
  const symbol_entry *l = lsv;
  return container_hash_chars(l->name);
}
HASHSET_DECLARE_STATIC_INLINE(hir_scope, symbol_entry,
                              container_cmp_hir_scope_entry, container_new_move,
                              container_delete_false,
                              container_hash_hir_scope_entry);

static inline void container_delete_scope(void *data) { hir_scope_free(data); }
LIST_DECLARE_STATIC_INLINE(list_hir_scope, hir_scope, container_cmp_false,
                           container_new_move, container_delete_scope);

typedef struct hir_ctx_struct {
  list_hir_scope *scopes;
  symbol_table   *symbol_table;
  list_exception *exceptions;
} hir_ctx;

static void hir_ctx_init(hir_ctx *ctx, symbol_table *symbol_table,
                         list_exception *exceptions) {
  ctx->scopes       = list_hir_scope_new();
  ctx->symbol_table = symbol_table;
  ctx->exceptions   = exceptions;
}

static void hir_ctx_deinit(hir_ctx *ctx) { list_hir_scope_free(ctx->scopes); }

static symbol_entry *hir_ctx_scopes_insert(hir_ctx *ctx, symbol_entry *entry) {
  hir_scope   *scope = list_hir_scope_front(ctx->scopes);
  hir_scope_it it    = hir_scope_insert(scope, entry);
  // debug("scope_size: %zu", scope->hashset.size);
  return GET(it);
}

static void hir_ctx_scopes_push(hir_ctx *ctx) {
  list_hir_scope_push_front(ctx->scopes, hir_scope_new());
  // debug("scopes_height: %zu", list_hir_scope_size(ctx->scopes));
}

static void hir_ctx_scopes_pop(hir_ctx *ctx) {
  hir_scope *scope = list_hir_scope_pop_front(ctx->scopes);
  ctx->scopes->list.f_delete(scope);
  // debug("scopes_height: %zu", list_hir_scope_size(ctx->scopes));
}

static inline symbol_entry *
hir_ctx_table_emplace(hir_ctx *ctx, char *id, type_entry *type, span *span) {
  return symbol_table_emplace(ctx->symbol_table, id, type, span);
}

static symbol_entry *hir_ctx_scopes_find(const hir_ctx *ctx, const char *id) {
  for (list_hir_scope_it it = list_hir_scope_begin(ctx->scopes); !END(it);
       NEXT(it)) {
    hir_scope   *scope = GET(it);
    hir_scope_it scope_it =
        hir_scope_find(scope, &(symbol_entry){.name = (char *)id});

    if (!END(scope_it)) {
      return GET(scope_it);
    }
  }
  return NULL;
}

static void hir_ctx_error(hir_ctx *ctx, exception_subtype_hir subtype,
                          const span *span, const char *format, ...) {

  va_list args;
  va_start(args, format);
  hir_exception_add_error_v(ctx->exceptions, subtype, span, format, args);
  va_end(args);
}

// visitor function
static int hir_bind_var(hir_ctx *ctx, hir_var *var) {
  symbol_entry *found;

  int is_bind_symbol = var->state & HIR_STATE_BIND_SYMBOL;
  int is_bind_type   = var->state & HIR_STATE_BIND_TYPE;

  if (!(is_bind_symbol)) {
    if ((found = hir_ctx_scopes_find(ctx, var->id_hir->name))) {
      hir_ctx_error(ctx, EXCEPTION_HIR_SYMBOL_REDEFINITION, var->base.span,
                    "symbol var name collision %s with %s declared at %s:%d:%d",
                    var->id_hir->name, found->name, found->span->source_ref,
                    found->span->line_start, found->span->line_end);
      return -1;
    }

    symbol_entry *entry = hir_ctx_table_emplace(
        ctx, strdup(var->id_hir->name), is_bind_type ? var->type_ref : NULL,
        var->base.span);

    hir_ctx_scopes_insert(ctx, entry);

    hir_id_free(var->id_hir);
    var->id_ref    = entry;
    var->base.span = NULL;
    var->state |= HIR_STATE_BIND_SYMBOL;
  }
  return 0;
}

static int hir_bind_param(hir_ctx *ctx, hir_param *param) {
  symbol_entry *found;

  int is_bind_symbol = param->state & HIR_STATE_BIND_SYMBOL;
  int is_bind_type   = param->state & HIR_STATE_BIND_TYPE;

  if (!(is_bind_symbol)) {
    if ((found = hir_ctx_scopes_find(ctx, param->id_hir->name))) {
      hir_ctx_error(
          ctx, EXCEPTION_HIR_SYMBOL_REDEFINITION, param->base.span,
          "symbol param name collision %s with %s declared at %s:%d:%d",
          param->id_hir->name, found->name, found->span->source_ref,
          found->span->line_start, found->span->line_end);
      return -1;
    }

    symbol_entry *entry = hir_ctx_table_emplace(
        ctx, strdup(param->id_hir->name), is_bind_type ? param->type_ref : NULL,
        param->base.span);

    hir_ctx_scopes_insert(ctx, entry);

    hir_id_free(param->id_hir);
    param->id_ref    = entry;
    param->base.span = NULL;
    param->state |= HIR_STATE_BIND_SYMBOL;
  }
  return 0;
}

static void hir_bind_subroutine_vars(hir_ctx *ctx, list_hir_var *vars) {
  if (!vars) {
    return;
  }

  for (list_hir_var_it it = list_hir_var_begin(vars); !END(it); NEXT(it)) {
    hir_var *var = GET(it);
    hir_bind_var(ctx, var);
  }
}

static void hir_bind_subroutine_body_expr(hir_ctx *ctx, hir_expr_base *base) {
  if (!base) {
    return;
  }

  switch (base->kind) {
    case HIR_EXPR_UNARY: {
      hir_expr_unary *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->first);
      return;
    }
    case HIR_EXPR_BINARY: {
      hir_expr_binary *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->first);
      hir_bind_subroutine_body_expr(ctx, self->second);
      return;
    }
    case HIR_EXPR_LITERAL: {
      return;
    }
    case HIR_EXPR_IDENTIFIER: {
      symbol_entry *found;
      hir_expr_id  *self = (typeof(self))base;

      if (!(self->base.state & HIR_STATE_BIND_SYMBOL)) {

        if (!(found = hir_ctx_scopes_find(ctx, self->id_hir->name))) {
          hir_ctx_error(ctx, EXCEPTION_HIR_SYMBOL_UNDEFINED,
                        self->id_hir->base.span, "symbol %s not found",
                        self->id_hir->name);
          return;
        }

        hir_id_free(self->id_hir);
        self->id_ref = found;
        self->base.state |= HIR_STATE_BIND_SYMBOL;
      }
      return;
    }
    case HIR_EXPR_CALL: {
      hir_expr_call *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->callee);
      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        hir_bind_subroutine_body_expr(ctx, GET(it));
      }
      return;
    }
    case HIR_EXPR_INDEX: {
      hir_expr_index *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->indexed);
      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        hir_bind_subroutine_body_expr(ctx, GET(it));
      }
      return;
    }
    case HIR_EXPR_BUILTIN: {
      hir_expr_builtin *self = (typeof(self))base;
      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        hir_bind_subroutine_body_expr(ctx, GET(it));
      }
      return;
    }
  }
  error("unexpected expr kind %d %p", base->kind, base);
}

static void hir_bind_subroutine_body_stmt(hir_ctx *ctx, hir_stmt_base *base) {
  if (!base) {
    return;
  }
  switch (base->kind) {
    case HIR_STMT_IF: {
      hir_stmt_if *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->cond);
      hir_bind_subroutine_body_stmt(ctx, self->je);
      hir_bind_subroutine_body_stmt(ctx, self->jz);
      return;
    }
    case HIR_STMT_BLOCK: {
      hir_stmt_block *self = (typeof(self))base;
      for (list_hir_stmt_it it = list_hir_stmt_begin(self->stmts); !END(it);
           NEXT(it)) {
        hir_bind_subroutine_body_stmt(ctx, GET(it));
      }
      return;
    }
    case HIR_STMT_WHILE: {
      hir_stmt_while *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->cond);
      hir_bind_subroutine_body_stmt(ctx, self->stmt);
      return;
    }
    case HIR_STMT_DO: {
      hir_stmt_do *self = (typeof(self))base;
      hir_bind_subroutine_body_stmt(ctx, self->stmt);
      hir_bind_subroutine_body_expr(ctx, self->cond);
      return;
    }
    case HIR_STMT_BREAK: {
      return;
    }
    case HIR_STMT_EXPR: {
      hir_stmt_expr *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->expr);
      return;
    }
    case HIR_STMT_RETURN: {
      hir_stmt_return *self = (typeof(self))base;
      hir_bind_subroutine_body_expr(ctx, self->expr);
      return;
    }
  }
  error("unexpected stmt kind %d %p", base->kind, base);
}

static void hir_bind_subroutine_signature(hir_ctx        *ctx,
                                          hir_subroutine *hir_subroutine) {
  symbol_entry *found;
  int           is_bind_symbol = hir_subroutine->state & HIR_STATE_BIND_SYMBOL;
  int           is_bind_type   = hir_subroutine->state & HIR_STATE_BIND_TYPE;

  // add subroutine to current scope
  if (!(is_bind_symbol)) {
    if ((found = hir_ctx_scopes_find(ctx, hir_subroutine->id_hir->name))) {
      hir_ctx_error(
          ctx, EXCEPTION_HIR_SYMBOL_REDEFINITION, hir_subroutine->base.span,
          "symbol subroutine name collision %s with %s declared at %s:%d:%d",
          hir_subroutine->id_hir->name, found->name, found->span->source_ref,
          found->span->line_start, found->span->line_end);
      return;
    }

    symbol_entry *entry =
        hir_ctx_table_emplace(ctx, strdup(hir_subroutine->id_hir->name),
                              is_bind_type ? hir_subroutine->type_ref : NULL,
                              hir_subroutine->base.span);

    hir_ctx_scopes_insert(ctx, entry);

    hir_id_free(hir_subroutine->id_hir);
    hir_subroutine->id_ref    = entry;
    hir_subroutine->base.span = NULL;
    hir_subroutine->state |= HIR_STATE_BIND_SYMBOL;
  }

  hir_ctx_scopes_push(ctx);

  for (list_hir_param_it it = list_hir_param_begin(hir_subroutine->params);
       !END(it); NEXT(it)) {
    hir_bind_param(ctx, GET(it));
  }

  hir_ctx_scopes_pop(ctx);
}

static void hir_bind_subroutine_body(hir_ctx        *ctx,
                                     hir_subroutine *hir_subroutine) {
  hir_subroutine_body *hir_body = hir_subroutine->body;

  if (!hir_body) {
    return;
  }

  switch (hir_body->kind) {
    case HIR_SUBROUTINE_BODY_IMPORT:
      return;
    case HIR_SUBROUTINE_BODY_BLOCK:
      hir_ctx_scopes_push(ctx);

      // add params again to scope
      for (list_hir_param_it it = list_hir_param_begin(hir_subroutine->params);
           !END(it); NEXT(it)) {
        hir_param *param = GET(it);

        if (!(param->state & HIR_STATE_BIND_SYMBOL)) {
          error("when binding body params should already be binded");
          continue;
        }

        hir_ctx_scopes_insert(ctx, param->id_ref);
      }

      hir_bind_subroutine_vars(ctx, hir_body->body.block.vars);
      hir_bind_subroutine_body_stmt(
          ctx, (hir_stmt_base *)hir_body->body.block.block);

      hir_ctx_scopes_pop(ctx);
      return;
  }
  error("unexpected body kind %d %p", hir_body->kind, hir_body);
}

static void hir_bind_class(hir_ctx *ctx, hir_class *hir_class) {
  hir_ctx_scopes_push(ctx);

  // instance vars and methods should be accessed using this.<smth>
  // to make it work the same way for all args
  hir_ctx_scopes_push(ctx);
  hir_bind_subroutine_vars(ctx, hir_class->fields);
  hir_ctx_scopes_pop(ctx);

  for (list_hir_method_it it = list_hir_method_begin(hir_class->methods);
       !END(it); NEXT(it)) {
    hir_method *method = GET(it);

    // update span for bind_subroutine to get relevant info about location
    span_free(method->subroutine->base.span);
    method->subroutine->base.span = method->base.span;
    method->base.span             = NULL;

    hir_bind_subroutine_signature(ctx, method->subroutine);
  }

  for (list_hir_method_it it = list_hir_method_begin(hir_class->methods);
       !END(it); NEXT(it)) {
    hir_method *method = GET(it);
    hir_bind_subroutine_body(ctx, method->subroutine);
  }

  hir_ctx_scopes_pop(ctx);
}

hir_bind_symbols_result hir_bind_symbols(hir *hir) {
  hir_bind_symbols_result result = {
      .symbol_table = symbol_table_new(),
      .exceptions   = list_exception_new(),
  };

  hir_ctx ctx;
  hir_ctx_init(&ctx, result.symbol_table, result.exceptions);

  // push global scope
  hir_ctx_scopes_push(&ctx);

  for (list_hir_subroutine_it it = list_hir_subroutine_begin(hir->subroutines);
       !END(it); NEXT(it)) {
    hir_subroutine *sub = GET(it);
    hir_bind_subroutine_signature(&ctx, sub);
  }

  for (list_hir_subroutine_it it = list_hir_subroutine_begin(hir->subroutines);
       !END(it); NEXT(it)) {
    hir_subroutine *sub = GET(it);
    hir_bind_subroutine_body(&ctx, sub);
  }

  for (list_hir_class_it it = list_hir_class_begin(hir->classes); !END(it);
       NEXT(it)) {
    hir_class *class = GET(it);
    hir_bind_class(&ctx, class);
  }

  hir_ctx_scopes_pop(&ctx);
  hir_ctx_deinit(&ctx);

  return result;
}
