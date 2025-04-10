#include "compiler/hir/node.h"
#include "compiler/span/str.h"
#include "compiler/type_table/str.h"
#include "str.h"

#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include <stdio.h>

#define HIR_STR_TAB 2
#define HIR_STR_SPACE " "

static void hir_tree_str_stmt_if(strbuf *buffer, size_t pad,
                                 const hir_stmt_if *stmt);
static void hir_tree_str_stmt_block(strbuf *buffer, size_t pad,
                                    const hir_stmt_block *stmt);
static void hir_tree_str_stmt_while(strbuf *buffer, size_t pad,
                                    const hir_stmt_while *stmt);
static void hir_tree_str_stmt_do(strbuf *buffer, size_t pad,
                                 const hir_stmt_do *stmt);
static void hir_tree_str_stmt_break(strbuf *buffer, size_t pad,
                                    const hir_stmt_break *stmt);
static void hir_tree_str_stmt_expr(strbuf *buffer, size_t pad,
                                   const hir_stmt_expr *stmt);
static void hir_tree_str_stmt_return(strbuf *buffer, size_t pad,
                                     const hir_stmt_return *stmt);

static void hir_tree_str_expr_unary(strbuf *buffer, size_t pad,
                                    const hir_expr_unary *expr);
static void hir_tree_str_expr_binary(strbuf *buffer, size_t pad,
                                     const hir_expr_binary *expr);
static void hir_tree_str_expr_lit(strbuf *buffer, size_t pad,
                                  const hir_expr_lit *expr);
static void hir_tree_str_expr_id(strbuf *buffer, size_t pad,
                                 const hir_expr_id *expr);
static void hir_tree_str_expr_call(strbuf *buffer, size_t pad,
                                   const hir_expr_call *expr);
static void hir_tree_str_expr_index(strbuf *buffer, size_t pad,
                                    const hir_expr_index *expr);
static void hir_tree_str_expr_builtin(strbuf *buffer, size_t pad,
                                      const hir_expr_builtin *expr);

static void strbuf_append_padding(strbuf *buffer, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    strbuf_append(buffer, HIR_STR_SPACE);
  }
}

static void strbuf_appendln(strbuf *buffer, const char *data) {
  strbuf_append(buffer, data);
  strbuf_append(buffer, "\n");
}

static void hir_tree_str_span(strbuf *buffer, size_t pad, const span *span) {
  UNUSED(pad);
  if (span) {
    char *s = span_str(span);
    strbuf_append(buffer, s);
    free(s);
  } else {
    strbuf_append(buffer, "(nil)");
  }
}

static void hir_tree_str_type(strbuf *buffer, size_t pad,
                              const hir_type_base *type) {
  UNUSED(pad);
  char *s = hir_type_str(type);
  if (s) {
    strbuf_append(buffer, s);
    free(s);
  } else {
    strbuf_append(buffer, "(nil)");
  }
}

static void hir_tree_str_symbol_entry(strbuf *buffer, size_t pad,
                                      const symbol_entry *entry_ref) {
  UNUSED(pad);
  char buf[64];
  strbuf_append(buffer, "symbol_table(");
  snprintf(buf, STRMAXLEN(buf), "%p", entry_ref);
  strbuf_append(buffer, buf);
  strbuf_append(buffer, ") // ");
  strbuf_append(buffer, entry_ref->name);
}

static void hir_tree_str_type_entry(strbuf *buffer, size_t pad,
                                    const type_entry *entry_ref) {
  UNUSED(pad);

  if (!entry_ref) {
    strbuf_append(buffer, "(nil)");
    return;
  }

  char buf[64];
  strbuf_append(buffer, "type_table(");
  snprintf(buf, STRMAXLEN(buf), "%p", entry_ref);
  strbuf_append(buffer, buf);
  strbuf_append(buffer, ")");
  char *type_s = type_str(entry_ref->type);
  if (type_s) {
    strbuf_append(buffer, " // ");
    strbuf_append(buffer, type_s);
    free(type_s);
  }
}

static void hir_tree_str_base(strbuf *buffer, size_t pad,
                              const hir_base *base) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "kind: ");
  strbuf_appendln(buffer, hir_node_enum_str(base->kind));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "span: ");
  hir_tree_str_span(buffer, pad, base->span);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_id(strbuf *buffer, size_t pad, const hir_id *id) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &id->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "name: ");
  strbuf_appendln(buffer, id->name);

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_lit(strbuf *buffer, size_t pad, const hir_lit *lit) {
  if (!lit) {
    strbuf_append(buffer, "(nil)");
    return;
  }

  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &lit->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "type: ");
  if (lit->state & HIR_STATE_BIND_TYPE) {
    hir_tree_str_type_entry(buffer, pad, lit->type_ref);
  } else {
    hir_tree_str_type(buffer, pad, lit->type_hir);
  }
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "value: ");
  char *s = hir_lit_value_str(lit);
  strbuf_appendln(buffer, s);
  free(s);

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}
static void hir_tree_str_param(strbuf *buffer, size_t pad,
                               const hir_param *param) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &param->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "state: ");
  strbuf_appendln(buffer, hir_state_enum_str(param->state));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "id: ");
  if (param->state & HIR_STATE_BIND_SYMBOL) {
    hir_tree_str_symbol_entry(buffer, pad, param->id_ref);
  } else {
    hir_tree_str_id(buffer, pad, param->id_hir);
  }
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "type: ");
  if (param->state & HIR_STATE_BIND_TYPE) {
    hir_tree_str_type_entry(buffer, pad, param->type_ref);
  } else {
    hir_tree_str_type(buffer, pad, param->type_hir);
  }
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_var(strbuf *buffer, size_t pad, const hir_var *var) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &var->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "state: ");
  strbuf_appendln(buffer, hir_state_enum_str(var->state));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "id: ");
  if (var->state & HIR_STATE_BIND_SYMBOL) {
    hir_tree_str_symbol_entry(buffer, pad, var->id_ref);
  } else {
    hir_tree_str_id(buffer, pad, var->id_hir);
  }
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "type: ");
  if (var->state & HIR_STATE_BIND_TYPE) {
    hir_tree_str_type_entry(buffer, pad, var->type_ref);
  } else {
    hir_tree_str_type(buffer, pad, var->type_hir);
  }
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr_base(strbuf *buffer, size_t pad,
                                   const hir_expr_base *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "kind: ");
  strbuf_appendln(buffer, hir_expr_enum_str(expr->kind));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "state: ");
  strbuf_appendln(buffer, hir_state_enum_str(expr->state));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "type: ");
  if (expr->state & HIR_STATE_BIND_TYPE) {
    hir_tree_str_type_entry(buffer, pad, expr->type_ref);
  } else {
    hir_tree_str_type(buffer, pad, expr->type_hir);
  }
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr(strbuf *buffer, size_t pad,
                              const hir_expr_base *expr) {
  if (!expr) {
    strbuf_append(buffer, "(nil)");
    return;
  }
  switch (expr->kind) {
    case HIR_EXPR_UNARY:
      return hir_tree_str_expr_unary(buffer, pad, (hir_expr_unary *)expr);
    case HIR_EXPR_BINARY:
      return hir_tree_str_expr_binary(buffer, pad, (hir_expr_binary *)expr);
    case HIR_EXPR_LITERAL:
      return hir_tree_str_expr_lit(buffer, pad, (hir_expr_lit *)expr);
    case HIR_EXPR_IDENTIFIER:
      return hir_tree_str_expr_id(buffer, pad, (hir_expr_id *)expr);
    case HIR_EXPR_CALL:
      return hir_tree_str_expr_call(buffer, pad, (hir_expr_call *)expr);
    case HIR_EXPR_INDEX:
      return hir_tree_str_expr_index(buffer, pad, (hir_expr_index *)expr);
    case HIR_EXPR_BUILTIN:
      return hir_tree_str_expr_builtin(buffer, pad, (hir_expr_builtin *)expr);
  }
  warn("unknown expr kind %d", expr->kind);
}

static void hir_tree_str_expr_unary(strbuf *buffer, size_t pad,
                                    const hir_expr_unary *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "op: ");
  strbuf_appendln(buffer, hir_expr_unary_enum_str(expr->op));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "first: ");
  hir_tree_str_expr(buffer, pad, expr->first);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr_binary(strbuf *buffer, size_t pad,
                                     const hir_expr_binary *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "op: ");
  strbuf_appendln(buffer, hir_expr_binary_enum_str(expr->op));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "first: ");
  hir_tree_str_expr(buffer, pad, expr->first);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "second: ");
  hir_tree_str_expr(buffer, pad, expr->second);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr_lit(strbuf *buffer, size_t pad,
                                  const hir_expr_lit *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "lit: ");
  hir_tree_str_lit(buffer, pad, expr->lit);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr_id(strbuf *buffer, size_t pad,
                                 const hir_expr_id *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "id: ");
  if (expr->base.state & HIR_STATE_BIND_SYMBOL) {
    hir_tree_str_symbol_entry(buffer, pad, expr->id_ref);
  } else {
    hir_tree_str_id(buffer, pad, expr->id_hir);
  }
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_list_expr(strbuf *buffer, size_t pad,
                                   const list_hir_expr *list) {
  strbuf_appendln(buffer, "[");
  pad += HIR_STR_TAB;
  for (list_hir_expr_it it = list_hir_expr_begin(list); !END(it); NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_expr(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "]");
}

static void hir_tree_str_expr_call(strbuf *buffer, size_t pad,
                                   const hir_expr_call *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "callee: ");
  hir_tree_str_expr(buffer, pad, expr->callee);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "args: ");
  hir_tree_str_list_expr(buffer, pad, expr->args);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr_index(strbuf *buffer, size_t pad,
                                    const hir_expr_index *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "indexed: ");
  hir_tree_str_expr(buffer, pad, expr->indexed);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "args: ");
  hir_tree_str_list_expr(buffer, pad, expr->args);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_expr_builtin(strbuf *buffer, size_t pad,
                                      const hir_expr_builtin *expr) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_expr_base(buffer, pad, &expr->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "kind: ");
  strbuf_append(buffer, hir_expr_builtin_enum_str(expr->kind));
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "args: ");
  hir_tree_str_list_expr(buffer, pad, expr->args);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_base(strbuf *buffer, size_t pad,
                                   const hir_stmt_base *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "kind: ");
  strbuf_appendln(buffer, hir_stmt_enum_str(stmt->kind));

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt(strbuf *buffer, size_t pad,
                              const hir_stmt_base *stmt) {
  if (!stmt) {
    strbuf_append(buffer, "(nil)");
    return;
  }
  switch (stmt->kind) {
    case HIR_STMT_IF:
      return hir_tree_str_stmt_if(buffer, pad, (hir_stmt_if *)stmt);
    case HIR_STMT_BLOCK:
      return hir_tree_str_stmt_block(buffer, pad, (hir_stmt_block *)stmt);
    case HIR_STMT_WHILE:
      return hir_tree_str_stmt_while(buffer, pad, (hir_stmt_while *)stmt);
    case HIR_STMT_DO:
      return hir_tree_str_stmt_do(buffer, pad, (hir_stmt_do *)stmt);
    case HIR_STMT_BREAK:
      return hir_tree_str_stmt_break(buffer, pad, (hir_stmt_break *)stmt);
    case HIR_STMT_EXPR:
      return hir_tree_str_stmt_expr(buffer, pad, (hir_stmt_expr *)stmt);
    case HIR_STMT_RETURN:
      return hir_tree_str_stmt_return(buffer, pad, (hir_stmt_return *)stmt);
  }
  warn("unknown stmt kind %d", stmt->kind);
}

static void hir_tree_str_stmt_if(strbuf *buffer, size_t pad,
                                 const hir_stmt_if *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "cond: ");
  hir_tree_str_expr(buffer, pad, stmt->cond);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "je: ");
  hir_tree_str_stmt(buffer, pad, stmt->je);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "jz: ");
  hir_tree_str_stmt(buffer, pad, stmt->jz);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_block(strbuf *buffer, size_t pad,
                                    const hir_stmt_block *stmt) {
  if (!stmt) {
    strbuf_append(buffer, "(nil)");
    return;
  }

  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "stmts: [");
  pad += HIR_STR_TAB;
  for (list_hir_stmt_it it = list_hir_stmt_begin(stmt->stmts); !END(it);
       NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_stmt(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "]");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_while(strbuf *buffer, size_t pad,
                                    const hir_stmt_while *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "cond: ");
  hir_tree_str_expr(buffer, pad, stmt->cond);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "stmt: ");
  hir_tree_str_stmt(buffer, pad, stmt->stmt);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_do(strbuf *buffer, size_t pad,
                                 const hir_stmt_do *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "positive: ");
  char buf[16];
  if (stmt->positive) {
    sprintf(buf, "true");
  } else {
    sprintf(buf, "false");
  }
  strbuf_appendln(buffer, buf);

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "cond: ");
  hir_tree_str_expr(buffer, pad, stmt->cond);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "stmt: ");
  hir_tree_str_stmt(buffer, pad, stmt->stmt);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_break(strbuf *buffer, size_t pad,
                                    const hir_stmt_break *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_expr(strbuf *buffer, size_t pad,
                                   const hir_stmt_expr *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "expr: ");
  hir_tree_str_expr(buffer, pad, stmt->expr);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_stmt_return(strbuf *buffer, size_t pad,
                                     const hir_stmt_return *stmt) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_stmt_base(buffer, pad, &stmt->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "expr: ");
  hir_tree_str_expr(buffer, pad, stmt->expr);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_subroutine_body(strbuf *buffer, size_t pad,
                                         const hir_subroutine_body *body) {

  if (!body) {
    strbuf_append(buffer, "(nil)");
    return;
  }

  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  switch (body->kind) {
    case HIR_SUBROUTINE_BODY_IMPORT:
      strbuf_append_padding(buffer, pad);
      strbuf_append(buffer, "lib: ");
      hir_tree_str_lit(buffer, pad, body->body.import.lib);
      strbuf_appendln(buffer, "");

      strbuf_append_padding(buffer, pad);
      strbuf_append(buffer, "entry: ");
      hir_tree_str_lit(buffer, pad, body->body.import.entry);
      strbuf_appendln(buffer, "");
      break;
    case HIR_SUBROUTINE_BODY_BLOCK:
      strbuf_append_padding(buffer, pad);
      strbuf_appendln(buffer, "vars: [");
      pad += HIR_STR_TAB;
      for (list_hir_var_it it = list_hir_var_begin(body->body.block.vars);
           !END(it); NEXT(it)) {
        strbuf_append_padding(buffer, pad);
        hir_tree_str_var(buffer, pad, GET(it));
        strbuf_appendln(buffer, "");
      }
      pad -= HIR_STR_TAB;
      strbuf_append_padding(buffer, pad);
      strbuf_appendln(buffer, "]");

      strbuf_append_padding(buffer, pad);
      strbuf_append(buffer, "block: ");
      hir_tree_str_stmt_block(buffer, pad, body->body.block.block);
      strbuf_appendln(buffer, "");
      break;
    default:
      warn("unknown subroutine body kind %d %p", body->kind, body);
      break;
  }

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_subroutine(strbuf *buffer, size_t pad,
                                    const hir_subroutine *subroutine) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &subroutine->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "state: ");
  strbuf_appendln(buffer, hir_state_enum_str(subroutine->state));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "id: ");
  if (subroutine->state & HIR_STATE_BIND_SYMBOL) {
    hir_tree_str_symbol_entry(buffer, pad, subroutine->id_ref);
  } else {
    hir_tree_str_id(buffer, pad, subroutine->id_hir);
  }
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "params: [");
  pad += HIR_STR_TAB;
  for (list_hir_param_it it = list_hir_param_begin(subroutine->params);
       !END(it); NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_param(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "]");

  if (subroutine->state & HIR_STATE_BIND_TYPE) {
    strbuf_append_padding(buffer, pad);
    strbuf_append(buffer, "type: ");
    hir_tree_str_type_entry(buffer, pad, subroutine->type_ref);
    strbuf_appendln(buffer, "");
  } else {
    strbuf_append_padding(buffer, pad);
    strbuf_append(buffer, "type_ret: ");
    hir_tree_str_type(buffer, pad, subroutine->type_ret);
    strbuf_appendln(buffer, "");
  }

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "spec: ");
  strbuf_append(buffer, hir_subroutine_spec_str(subroutine->spec));
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "body: ");
  hir_tree_str_subroutine_body(buffer, pad, subroutine->body);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_method(strbuf *buffer, size_t pad,
                                const hir_method *method) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &method->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "modifier: ");
  strbuf_appendln(buffer, hir_method_modifier_enum_str(method->modifier));

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "subroutine: ");
  hir_tree_str_subroutine(buffer, pad, method->subroutine);
  strbuf_appendln(buffer, "");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

static void hir_tree_str_class(strbuf *buffer, size_t pad,
                               const hir_class *class) {
  strbuf_appendln(buffer, "{");
  pad += HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "base: ");
  hir_tree_str_base(buffer, pad, &class->base);
  strbuf_appendln(buffer, "");

  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "state: ");
  strbuf_appendln(buffer, hir_state_enum_str(class->state));

  if (class->state & HIR_STATE_BIND_TYPE) {
    strbuf_append_padding(buffer, pad);
    strbuf_append(buffer, "type: ");
    hir_tree_str_type_entry(buffer, pad, class->type_ref);
    strbuf_appendln(buffer, "");
  } else {
    strbuf_append_padding(buffer, pad);
    strbuf_append(buffer, "id: ");
    hir_tree_str_id(buffer, pad, class->id);
    strbuf_appendln(buffer, "");

    strbuf_append_padding(buffer, pad);
    strbuf_appendln(buffer, "typenames: [");
    pad += HIR_STR_TAB;
    for (list_hir_id_it it = list_hir_id_begin(class->typenames); !END(it);
         NEXT(it)) {
      strbuf_append_padding(buffer, pad);
      hir_tree_str_id(buffer, pad, GET(it));
      strbuf_appendln(buffer, "");
    }
    pad -= HIR_STR_TAB;
    strbuf_append_padding(buffer, pad);
    strbuf_appendln(buffer, "]");

    strbuf_append_padding(buffer, pad);
    strbuf_appendln(buffer, "parents: [");
    pad += HIR_STR_TAB;
    for (list_hir_type_it it = list_hir_type_begin(class->parents); !END(it);
         NEXT(it)) {
      strbuf_append_padding(buffer, pad);
      hir_tree_str_type(buffer, pad, GET(it));
      strbuf_appendln(buffer, "");
    }
    pad -= HIR_STR_TAB;
    strbuf_append_padding(buffer, pad);
    strbuf_appendln(buffer, "]");
  }

  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "fields: [");
  pad += HIR_STR_TAB;
  for (list_hir_var_it it = list_hir_var_begin(class->fields); !END(it);
       NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_var(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "]");

  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "methods: [");
  pad += HIR_STR_TAB;
  for (list_hir_method_it it = list_hir_method_begin(class->methods); !END(it);
       NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_method(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "]");

  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, pad);
  strbuf_append(buffer, "}");
}

char *hir_tree_str(const hir *hir) {
  strbuf *buffer = strbuf_new(0, 0);
  strbuf_appendln(buffer, "{");
  size_t pad = HIR_STR_TAB;

  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "classes: [");
  pad += HIR_STR_TAB;
  for (list_hir_class_it it = list_hir_class_begin(hir->classes); !END(it);
       NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_class(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, HIR_STR_TAB);
  strbuf_appendln(buffer, "]");

  strbuf_append_padding(buffer, pad);
  strbuf_appendln(buffer, "subroutines: [");
  pad += HIR_STR_TAB;
  for (list_hir_subroutine_it it = list_hir_subroutine_begin(hir->subroutines);
       !END(it); NEXT(it)) {
    strbuf_append_padding(buffer, pad);
    hir_tree_str_subroutine(buffer, pad, GET(it));
    strbuf_appendln(buffer, "");
  }
  pad -= HIR_STR_TAB;
  strbuf_append_padding(buffer, HIR_STR_TAB);
  strbuf_appendln(buffer, "]");

  strbuf_append(buffer, "}");
  return strbuf_detach(buffer);
}
