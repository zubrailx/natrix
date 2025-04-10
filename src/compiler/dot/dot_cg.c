#include "dot.h"

#include "util/hashset.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"

// SYM_SUB
typedef struct dot_sym_sub_struct {
  const symbol_entry   *sym;
  const mir_subroutine *sub;
} dot_sym_sub;

static dot_sym_sub *dot_sym_sub_new(const symbol_entry   *sym,
                                    const mir_subroutine *sub) {
  dot_sym_sub *self = MALLOC(dot_sym_sub);
  self->sym         = sym;
  self->sub         = sub;
  return self;
}

static inline int container_cmp_dot_sym_sub(const void *lsv, const void *rsv) {
  const dot_sym_sub *l = lsv;
  const dot_sym_sub *r = rsv;
  return container_cmp_ptr(l->sym, r->sym);
}
static inline void container_delete_dot_sym_sub(void *data) {
  container_delete_ptr(data);
}
static inline size_t container_hash_dot_sym_sub(const void *lsv) {
  const dot_sym_sub *l = lsv;
  return container_hash_ptr(l->sym);
}

HASHSET_DECLARE_STATIC_INLINE(hashset_dot_sym_sub, dot_sym_sub,
                              container_cmp_dot_sym_sub, container_new_move,
                              container_delete_dot_sym_sub,
                              container_hash_dot_sym_sub);

// DOT_SUB
typedef struct dot_sub_struct {
  const mir_subroutine *sub;
  size_t                id;
} dot_sub;

static dot_sub *dot_sub_new(const mir_subroutine *sub, size_t id) {
  dot_sub *self = MALLOC(dot_sub);
  self->sub     = sub;
  self->id      = id;
  return self;
}

static inline int container_cmp_dot_sub(const void *lsv, const void *rsv) {
  const dot_sub *l = lsv;
  const dot_sub *r = rsv;
  return container_cmp_ptr(l->sub, r->sub);
}
static inline void container_delete_dot_sub(void *data) {
  container_delete_ptr(data);
}
static inline size_t container_hash_dot_sub(const void *lsv) {
  const dot_sub *l = lsv;
  return container_hash_ptr(l->sub);
}

HASHSET_DECLARE_STATIC_INLINE(hashset_dot_sub, dot_sub, container_cmp_dot_sub,
                              container_new_move, container_delete_dot_sub,
                              container_hash_dot_sub);

// CTX
typedef struct dot_ctx_struct {
  size_t               sub_cnt;
  hashset_dot_sym_sub *map_sym_sub;
  hashset_dot_sub     *visited_subs;
  strbuf              *buffer;
} dot_ctx;

static void dot_ctx_init(dot_ctx *ctx, strbuf *buffer) {
  ctx->sub_cnt      = 0;
  ctx->map_sym_sub  = hashset_dot_sym_sub_new();
  ctx->visited_subs = hashset_dot_sub_new();
  ctx->buffer       = buffer;
}

static void dot_ctx_deinit(dot_ctx *ctx) {
  ctx->sub_cnt = 0;
  hashset_dot_sym_sub_free(ctx->map_sym_sub);
  hashset_dot_sub_free(ctx->visited_subs);
  ctx->buffer = NULL;
}

static void dot_ctx_setup(dot_ctx *ctx, const mir *mir) {
  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->defined_subs);
       !END(it); NEXT(it)) {
    mir_subroutine *sub = GET(it);
    hashset_dot_sym_sub_insert(ctx->map_sym_sub,
                               dot_sym_sub_new(sub->symbol_ref, sub));
  }

  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->declared_subs);
       !END(it); NEXT(it)) {
    mir_subroutine *sub = GET(it);
    hashset_dot_sym_sub_insert(ctx->map_sym_sub,
                               dot_sym_sub_new(sub->symbol_ref, sub));
  }

  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->imported_subs);
       !END(it); NEXT(it)) {
    mir_subroutine *sub = GET(it);
    hashset_dot_sym_sub_insert(ctx->map_sym_sub,
                               dot_sym_sub_new(sub->symbol_ref, sub));
  }
}

static dot_sub *dot_cg_traverse(dot_ctx *ctx, const mir_subroutine *sub) {
  char buf[256];

  if (!sub) {
    return NULL;
  }

  hashset_dot_sub_it it = hashset_dot_sub_find(
      ctx->visited_subs, &(dot_sub){.sub = (mir_subroutine *)sub});

  if (!END(it)) {
    return GET(it);
  }

  dot_sub *dot_cur = dot_sub_new(sub, ctx->sub_cnt++);
  hashset_dot_sub_insert(ctx->visited_subs, dot_cur);

  strbuf_append_f(ctx->buffer, buf, "subroutine_%lu[label=", dot_cur->id);
  strbuf_append(ctx->buffer, "< <FONT POINT-SIZE=\"16\"><b>");
  strbuf_append(ctx->buffer, sub->symbol_ref->name);
  strbuf_append(ctx->buffer, "</b></FONT><br/><FONT POINT-SIZE=\"12\">");
  switch (sub->kind) {
    case MIR_SUBROUTINE_DEFINED:
      strbuf_append(ctx->buffer, "defined");
      break;
    case MIR_SUBROUTINE_DECLARED:
      strbuf_append(ctx->buffer, "declared");
      break;
    case MIR_SUBROUTINE_IMPORTED:
      strbuf_append(ctx->buffer, "from \"");
      strbuf_append(ctx->buffer, sub->imported.lib);
      strbuf_append(ctx->buffer, "\"");
      if (sub->imported.entry) {
        strbuf_append(ctx->buffer, "in \"");
        strbuf_append(ctx->buffer, sub->imported.entry);
        strbuf_append(ctx->buffer, "\"");
      }
      break;
  }
  strbuf_append(ctx->buffer, "</FONT> >");
  strbuf_append(ctx->buffer, "];\n");

  if (sub->kind != MIR_SUBROUTINE_DEFINED) {
    return dot_cur;
  }

  for (list_mir_bb_it it = list_mir_bb_begin(sub->defined.bbs); !END(it);
       NEXT(it)) {
    mir_bb *bb = GET(it);

    for (list_mir_stmt_it it = list_mir_stmt_begin(bb->stmts); !END(it);
         NEXT(it)) {
      mir_stmt *stmt = GET(it);

      if (stmt->kind != MIR_STMT_CALL) {
        continue;
      }

      dot_sub *dot_child = dot_cg_traverse(ctx, stmt->call.sub);
      if (!dot_child) {
        warn("child sub %p traverse returned null");
        continue;
      }

      strbuf_append_f(ctx->buffer, buf, "subroutine_%lu -> subroutine_%lu;\n",
                      dot_cur->id, dot_child->id);
    }
  }
  return dot_cur;
}

dot_string *dot_cg(const mir *mir, const type_table *type_table,
                   const symbol_table   *symbol_table,
                   const mir_subroutine *sub) {
  UNUSED(mir);
  UNUSED(type_table);
  UNUSED(symbol_table);
  UNUSED(sub);

  strbuf *buffer = strbuf_new(0, 0);

  dot_ctx ctx;
  dot_ctx_init(&ctx, buffer);
  dot_ctx_setup(&ctx, mir);

  strbuf_append(
      buffer,
      "strict digraph G {\n"
      "compound=true;\n"
      "graph [pad=\"0\",nodesep=\"1\",ranksep=\"0.75\",splines=false];\n"
      "node [shape=rect, style=filled];\n"
      "\n");

  dot_cg_traverse(&ctx, sub);

  strbuf_append(buffer, "}");

  dot_ctx_deinit(&ctx);

  char *chars = strbuf_detach(buffer);
  return dot_string_new(chars, strlen(chars));
}
