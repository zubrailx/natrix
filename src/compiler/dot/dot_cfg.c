#include "compiler/hir/str.h"
#include "compiler/type_table/str.h"
#include "dot.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include "util/string.h"

// CTX
typedef struct dot_ctx_struct {
  size_t  expr_cnt;
  strbuf *buffer;
  int     option_add_expr;
  mir_bb *bb_ref;
} dot_ctx;

static void dot_ctx_init(dot_ctx *ctx, strbuf *buffer, int option_add_expr) {
  ctx->expr_cnt        = 0;
  ctx->buffer          = buffer;
  ctx->option_add_expr = option_add_expr;
  ctx->bb_ref          = NULL;
}

static void dot_ctx_deinit(dot_ctx *ctx) {
  ctx->buffer          = NULL;
  ctx->option_add_expr = 0;
}

static size_t dot_cfg_traverse_expr_node(dot_ctx             *ctx,
                                         const hir_expr_base *expr) {
  char buf[256];

  if (!expr) {
    return 0;
  }

  size_t expr_id = ctx->expr_cnt++;

  // construct current node label
  strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu[", ctx->bb_ref->id, expr_id);

  switch (expr->kind) {
    case HIR_EXPR_UNARY: {
      hir_expr_unary *self = (typeof(self))expr;
      strbuf_append_f(ctx->buffer, buf, "label=\"%s\"",
                      hir_expr_unary_enum_str(self->op));
      break;
    }
    case HIR_EXPR_BINARY: {
      hir_expr_binary *self = (typeof(self))expr;
      strbuf_append_f(ctx->buffer, buf, "label=\"%s\"",
                      hir_expr_binary_enum_str(self->op));
      break;
    }
    case HIR_EXPR_LITERAL: {
      hir_expr_lit *self  = (typeof(self))expr;
      char         *lit_s = hir_lit_value_str(self->lit);
      if (lit_s) {
        char *escaped_s = escape_html(lit_s);
        strbuf_append_f(ctx->buffer, buf, "label=\"%s\"", escaped_s);
        free(escaped_s);
        free(lit_s);
      }
      break;
    }
    case HIR_EXPR_IDENTIFIER: {
      hir_expr_id *self = (typeof(self))expr;
      strbuf_append_f(ctx->buffer, buf, "label=\"%s\"", self->id_ref->name);
      break;
    }
    case HIR_EXPR_CALL: {
      hir_expr_call *self = (typeof(self))expr;
      strbuf_append_f(ctx->buffer, buf, "label=\"%s\"",
                      hir_expr_enum_str(expr->kind));
      break;
    }
    case HIR_EXPR_INDEX: {
      hir_expr_index *self = (typeof(self))expr;
      strbuf_append_f(ctx->buffer, buf, "label=\"%s\"",
                      hir_expr_enum_str(expr->kind));
      break;
    }
    case HIR_EXPR_BUILTIN: {
      hir_expr_builtin *self = (typeof(self))expr;
      strbuf_append_f(ctx->buffer, buf, "label=\"%s_%s",
                      hir_expr_enum_str(expr->kind),
                      hir_expr_builtin_enum_str(self->kind));
      if (self->base.type_ref) {
        strbuf_append(ctx->buffer, "<");
        char *type_s = type_str(self->base.type_ref->type);
        if (type_s) {
          strbuf_append(ctx->buffer, type_s);
          free(type_s);
        }
        strbuf_append(ctx->buffer, ">");
      }
      strbuf_append(ctx->buffer, "\"");
      break;
    }
    default:
      warn("unhandled expr kind %d", expr->kind);
      break;
  }

  strbuf_append(ctx->buffer, "];\n");

  // traverse
  switch (expr->kind) {
    case HIR_EXPR_UNARY: {
      hir_expr_unary *self     = (typeof(self))expr;
      size_t          first_id = dot_cfg_traverse_expr_node(ctx, self->first);

      strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                      ctx->bb_ref->id, expr_id, ctx->bb_ref->id, first_id);
      break;
    }
    case HIR_EXPR_BINARY: {
      hir_expr_binary *self     = (typeof(self))expr;
      size_t           first_id = dot_cfg_traverse_expr_node(ctx, self->first);
      size_t second_id          = dot_cfg_traverse_expr_node(ctx, self->second);

      strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                      ctx->bb_ref->id, expr_id, ctx->bb_ref->id, first_id);
      strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                      ctx->bb_ref->id, expr_id, ctx->bb_ref->id, second_id);
      break;
    }
    case HIR_EXPR_LITERAL: {
      break;
    }
    case HIR_EXPR_IDENTIFIER: {
      break;
    }
    case HIR_EXPR_CALL: {
      hir_expr_call *self = (typeof(self))expr;

      size_t callee_id = dot_cfg_traverse_expr_node(ctx, self->callee);
      strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                      ctx->bb_ref->id, expr_id, ctx->bb_ref->id, callee_id);

      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        hir_expr_base *arg    = GET(it);
        size_t         arg_id = dot_cfg_traverse_expr_node(ctx, arg);
        strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                        ctx->bb_ref->id, expr_id, ctx->bb_ref->id, arg_id);
      }
      break;
    }
    case HIR_EXPR_INDEX: {
      hir_expr_index *self = (typeof(self))expr;

      size_t indexed_id = dot_cfg_traverse_expr_node(ctx, self->indexed);
      strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                      ctx->bb_ref->id, expr_id, ctx->bb_ref->id, indexed_id);

      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        hir_expr_base *arg    = GET(it);
        size_t         arg_id = dot_cfg_traverse_expr_node(ctx, arg);
        strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                        ctx->bb_ref->id, expr_id, ctx->bb_ref->id, arg_id);
      }
      break;
    }
    case HIR_EXPR_BUILTIN: {
      hir_expr_builtin *self = (typeof(self))expr;

      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        hir_expr_base *arg    = GET(it);
        size_t         arg_id = dot_cfg_traverse_expr_node(ctx, arg);
        strbuf_append_f(ctx->buffer, buf, "expr_%zu_%zu -> expr_%zu_%zu;\n",
                        ctx->bb_ref->id, expr_id, ctx->bb_ref->id, arg_id);
      }
      break;
    }
    default:
      warn("unhandled expr kind %d", expr->kind);
      break;
  }

  return expr_id;
}

static size_t dot_cfg_traverse_expr(dot_ctx *ctx, const hir_expr_base *expr) {
  char buf[64];

  strbuf_append(ctx->buffer, "subgraph ");
  strbuf_append_f(ctx->buffer, buf, "cluster_%zu_expr_%lu", ctx->bb_ref->id,
                  ctx->expr_cnt++);
  strbuf_append(ctx->buffer, "{\n"
                             "style=filled;\n"
                             "fillcolor=white;\n"
                             "\n");

  size_t expr_id = 0;
  if (expr) {
    expr_id = dot_cfg_traverse_expr_node(ctx, expr);
  }

  strbuf_append(ctx->buffer, "}\n");

  if (expr_id) {
    strbuf_append_f(ctx->buffer, buf,
                    "block_%lu -> expr_%lu_%lu[style=invis];\n",
                    ctx->bb_ref->id, ctx->bb_ref->id, expr_id);
  }
  return expr_id;
}

static void dot_cfg_traverse_bb_edges(dot_ctx *ctx, mir_bb *bb) {
  char buf[256];

  switch (mir_bb_get_cond(bb)) {
    case MIR_BB_COND: {
      const mir_bb *bb_je = bb->jmp.je_ref;

      strbuf_append_f(ctx->buffer, buf, "block_%zu -> block_%zu", bb->id,
                      bb_je->id);
      strbuf_append_f(
          ctx->buffer, buf,
          "[color=red,ltail=cluster_%zu,lhead=cluster_%zu,labelfloat=true,"
          "taillabel=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" "
          "CELLSPACING=\"0\"><TR><TD "
          "BGCOLOR=\"white\">True</TD></TR></TABLE>>];\n",
          bb->id, bb_je->id);

      const mir_bb *bb_jz = bb->jmp.jz_ref;
      strbuf_append_f(ctx->buffer, buf, "block_%zu -> block_%zu", bb->id,
                      bb_jz->id);
      strbuf_append_f(
          ctx->buffer, buf,
          "[color=blue,ltail=cluster_%zu,lhead=cluster_%zu,labelfloat=true,"
          "taillabel=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" "
          "CELLSPACING=\"0\"><TR><TD "
          "BGCOLOR=\"white\">False</TD></TR></TABLE>>];\n",
          bb->id, bb_jz->id);
      break;
    }
    case MIR_BB_NEXT: {
      const mir_bb *bb_next = bb->jmp.next_ref;

      strbuf_append_f(ctx->buffer, buf, "block_%zu -> block_%zu", bb->id,
                      bb_next->id);
      strbuf_append_f(
          ctx->buffer, buf,
          "[ltail=cluster_%zu,lhead=cluster_%zu,labelfloat=true];\n", bb->id,
          bb_next->id);
      break;
    }
    case MIR_BB_TERM:
      break;
    case MIR_BB_UNKNOWN:
      warn("unexpected block kind %d %p", mir_bb_get_cond(bb), bb);
      break;
  }
}

static void dot_cfg_traverse(dot_ctx *ctx, const mir_subroutine *sub) {
  char buf[256];

  if (sub->kind != MIR_SUBROUTINE_DEFINED) {
    return;
  }

  for (list_mir_bb_it it = list_mir_bb_begin(sub->defined.bbs); !END(it);
       NEXT(it)) {
    mir_bb *bb  = GET(it);
    ctx->bb_ref = bb;

    char label[64];
    if (list_mir_bb_front(sub->defined.bbs) == bb) {
      snprintf(label, STRMAXLEN(label), "block_%lu (START)", bb->id);
    } else if (list_mir_bb_back(sub->defined.bbs) == bb) {
      snprintf(label, STRMAXLEN(label), "block_%lu (END)", bb->id);
    } else {
      snprintf(label, STRMAXLEN(label), "block_%lu", bb->id);
    }

    strbuf_append_f(ctx->buffer, buf, "subgraph cluster_%lu {\n", bb->id);
    strbuf_append(ctx->buffer, "style=\"filled,solid\";\n"
                               "shape=box;\n"
                               "color=black;\n"
                               "fillcolor=lightgray;\n"
                               "\n");
    strbuf_append_f(ctx->buffer, buf, "block_%lu", bb->id);
    strbuf_append_f(ctx->buffer, buf,
                    "[shape=plaintext, label=\"%s\"];\n"
                    "\n",
                    label);

    if (ctx->option_add_expr) {
      ctx->expr_cnt = 1;

      size_t prev_id = 0;
      for (list_hir_expr_ref_it it = list_hir_expr_ref_begin(bb->hir_exprs);
           !END(it); NEXT(it)) {
        const hir_expr_base *expr    = GET(it);
        size_t               next_id = dot_cfg_traverse_expr(ctx, expr);

        if (prev_id) {
          strbuf_append_f(ctx->buffer, buf,
                          "expr_%lu_%lu -> expr_%lu_%lu[constraint=false];\n",
                          bb->id, prev_id, bb->id, next_id);
        }

        strbuf_append(ctx->buffer, "\n");

        prev_id = next_id;
      }
    }

    strbuf_append(ctx->buffer, "}\n\n");

    dot_cfg_traverse_bb_edges(ctx, bb);

    strbuf_append(ctx->buffer, "\n");
  }
}

dot_string *dot_cfg(const mir *mir, const hir *hir,
                    const type_table   *type_table,
                    const symbol_table *symbol_table, const mir_subroutine *sub,
                    int option_add_expr) {
  UNUSED(mir);
  UNUSED(hir);
  UNUSED(type_table);
  UNUSED(symbol_table);

  strbuf *buffer = strbuf_new(0, 0);

  dot_ctx ctx;
  dot_ctx_init(&ctx, buffer, option_add_expr);

  strbuf_append(buffer, "strict digraph G {\n"
                        "compound=true;\n"
                        "graph [nodesep=\"1\",ranksep=\"0.75\",splines=true];\n"
                        "node [shape=rect, style=filled];\n"
                        "\n");

  dot_cfg_traverse(&ctx, sub);

  strbuf_append(buffer, "}");

  dot_ctx_deinit(&ctx);

  char *chars = strbuf_detach(buffer);
  return dot_string_new(chars, strlen(chars));
}
