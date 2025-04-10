#include "merge_bb.h"

#include "compiler/exception/list.h"
#include "util/hashset.h"
#include "util/log.h"
#include "util/macro.h"

LIST_DECLARE_STATIC_INLINE(list_mir_bb_ref, mir_bb, container_cmp_false,
                           container_new_move, container_delete_false);

HASHSET_DECLARE_STATIC_INLINE(hashset_mir_bb_ref, mir_bb, container_cmp_ptr,
                              container_new_move, container_delete_false,
                              container_hash_ptr);

// MIR_BB_PREDS
typedef struct mir_bb_preds_struct {
  const mir_bb    *bb;
  list_mir_bb_ref *preds;
} mir_bb_preds;

mir_bb_preds *mir_bb_preds_new(const mir_bb *bb) {
  mir_bb_preds *self = MALLOC(mir_bb_preds);
  self->bb           = bb;
  self->preds        = list_mir_bb_ref_new();
  return self;
}

static inline int container_cmp_mir_bb_preds(const void *_lsv,
                                             const void *_rsv) {
  const mir_bb_preds *lsv = _lsv;
  const mir_bb_preds *rsv = _rsv;
  return container_cmp_ptr(lsv->bb, rsv->bb);
}
static inline void container_delete_mir_bb_preds(void *data) {
  mir_bb_preds *self = data;
  list_mir_bb_ref_free(self->preds);
  free(self);
}
static inline size_t container_hash_mir_bb_preds(const void *data) {
  const mir_bb_preds *self = data;
  return container_hash_ptr(self->bb);
}

HASHSET_DECLARE_STATIC_INLINE(hashset_mir_bb_preds, mir_bb_preds,
                              container_cmp_mir_bb_preds, container_new_move,
                              container_delete_mir_bb_preds,
                              container_hash_mir_bb_preds);

// CTX
typedef struct mir_ctx_struct {
  hashset_mir_bb_preds *preds;
  hashset_mir_bb_ref   *visited;
  hashset_mir_bb_ref   *deleted;

  list_exception *exceptions;
} mir_ctx;

static mir_bb_preds *mir_ctx_get_preds(mir_ctx *ctx, const mir_bb *cur) {
  if (!cur) {
    return NULL;
  }

  hashset_mir_bb_ref_it visited_it =
      hashset_mir_bb_ref_find(ctx->visited, (mir_bb *)cur);

  // if already traversed than return its predecessor list
  if (!END(visited_it)) {
    hashset_mir_bb_preds_it it =
        hashset_mir_bb_preds_find(ctx->preds, &(mir_bb_preds){.bb = cur});
    return GET(it);
  }

  hashset_mir_bb_ref_insert(ctx->visited, (mir_bb *)cur);

  mir_bb_preds *preds = mir_bb_preds_new(cur);
  hashset_mir_bb_preds_insert(ctx->preds, preds);

  if (mir_bb_get_cond(cur) != MIR_BB_TERM) {
    mir_bb_preds *je_preds = mir_ctx_get_preds(ctx, cur->jmp.je_ref);
    if (je_preds) {
      list_mir_bb_ref_push_back(je_preds->preds, (mir_bb *)cur);
    }

    mir_bb_preds *jz_preds = mir_ctx_get_preds(ctx, cur->jmp.jz_ref);
    if (jz_preds) {
      list_mir_bb_ref_push_back(jz_preds->preds, (mir_bb *)cur);
    }
  }

  return preds;
}

// returns next block that can be merged
// conditions (to be merged):
//   1. block is NEXT block
//   1.1. is empty -> no changes will be done to next block
//   1.2. next block has 1 reference (only cur block)
// (these references are from not deleted blocks, because no dead code
// optimizations are applied)
static mir_bb *mir_ctx_can_merge_with(mir_ctx *ctx, mir_bb *cur) {
  switch (mir_bb_get_cond(cur)) {
    case MIR_BB_COND:
    case MIR_BB_TERM:
      return NULL;
    case MIR_BB_NEXT: {

      if (list_mir_stmt_empty(cur->stmts) &&
          list_hir_expr_ref_empty(cur->hir_exprs)) {
        return cur->jmp.next_ref;
      }

      hashset_mir_bb_preds_it it = hashset_mir_bb_preds_find(
          ctx->preds, &(mir_bb_preds){.bb = cur->jmp.next_ref});

      if (list_mir_bb_ref_size(GET(it)->preds) <= 1) {
        return cur->jmp.next_ref;
      }

      return NULL;
    }
    default:
      error("unhandled mir bb kind %d", mir_bb_get_cond(cur));
      return NULL;
  }
}

static mir_bb *mir_ctx_bb_merge_block(mir_ctx *ctx, mir_bb *cur) {
  UNUSED(ctx);

  if (!cur) {
    return NULL;
  }

  mir_bb *next = mir_ctx_can_merge_with(ctx, cur);
  if (!next) {
    return NULL;
  }

  hashset_mir_bb_preds_it cur_preds_it =
      hashset_mir_bb_preds_find(ctx->preds, &(mir_bb_preds){.bb = cur});

  mir_bb_preds *cur_preds = GET(cur_preds_it);

  // update all predecessors
  for (list_mir_bb_ref_it it = list_mir_bb_ref_begin(cur_preds->preds);
       !END(it); NEXT(it)) {
    mir_bb *pred = GET(it);

    // check if not already deleted
    hashset_mir_bb_ref_it it = hashset_mir_bb_ref_find(ctx->deleted, pred);
    if (!END(it)) {
      continue;
    }

    hashset_mir_bb_preds_it next_it =
        hashset_mir_bb_preds_find(ctx->preds, &(mir_bb_preds){.bb = next});
    mir_bb_preds *next_preds = GET(next_it);

    if (pred->jmp.je_ref == cur) {
      pred->jmp.je_ref = next;
      // debug("upd.. %zu.je_ref -> %zu", pred_bb->id, next_bb->id);
      list_mir_bb_ref_push_back(next_preds->preds, pred);
    }
    if (pred->jmp.jz_ref == cur) {
      pred->jmp.jz_ref = next;
      // debug("upd.. %zu.jz_ref -> %zu", pred_bb->id, next_bb->id);
      list_mir_bb_ref_push_back(next_preds->preds, pred);
    }
  }

  // merge stmts
  list_mir_stmt *rev_stmt = list_mir_stmt_new();
  while (!list_mir_stmt_empty(cur->stmts)) {
    mir_stmt *stmt = list_mir_stmt_pop_front(cur->stmts);
    list_mir_stmt_push_front(rev_stmt, stmt);
  }
  while (!list_mir_stmt_empty(rev_stmt)) {
    mir_stmt *stmt = list_mir_stmt_pop_front(rev_stmt);
    list_mir_stmt_push_front(next->stmts, stmt);
  }
  list_mir_stmt_free(rev_stmt);

  // merge hir_exprs
  list_hir_expr_ref *rev_exprs = list_hir_expr_ref_new();
  while (!list_hir_expr_ref_empty(cur->hir_exprs)) {
    hir_expr_base *expr = list_hir_expr_ref_pop_front(cur->hir_exprs);
    list_hir_expr_ref_push_front(rev_exprs, expr);
  }
  while (!list_hir_expr_ref_empty(rev_exprs)) {
    hir_expr_base *expr = list_hir_expr_ref_pop_front(rev_exprs);
    list_hir_expr_ref_push_front(next->hir_exprs, expr);
  }
  list_hir_expr_ref_free(rev_exprs);

  return next;
}

static void mir_ctx_bb_merge_subroutine(mir_ctx *ctx, mir_subroutine *sub) {
  if (!list_mir_bb_size(sub->defined.bbs)) {
    return;
  }

  ctx->preds   = hashset_mir_bb_preds_new();
  ctx->visited = hashset_mir_bb_ref_new();
  ctx->deleted = hashset_mir_bb_ref_new();

  // calculate predicates for all bbs
  for (list_mir_bb_it it = list_mir_bb_begin(sub->defined.bbs); !END(it);
       NEXT(it)) {
    mir_ctx_get_preds(ctx, GET(it));
  }

  // check
  // for (hashset_mir_bb_preds_it it = hashset_mir_bb_preds_begin(ctx->preds);
  //      !END(it); NEXT(it)) {
  //   mir_bb_preds *bb_preds = GET(it);

  //   debug("bb%zu", bb_preds->bb->id);
  //   for (list_mir_bb_ref_it pred_it = list_mir_bb_ref_begin(bb_preds->preds);
  //        !END(pred_it); NEXT(pred_it)) {
  //     debug("  bb%zu", GET(pred_it)->id);
  //   }
  // }

  list_mir_bb *new_bbs = list_mir_bb_new();

  // bb_first may be redundant (need to check)
  while (!list_mir_bb_empty(sub->defined.bbs)) {
    mir_bb *cur      = list_mir_bb_pop_front(sub->defined.bbs);
    mir_bb *cur_next = mir_ctx_bb_merge_block(ctx, cur);
    if (cur_next) {
      hashset_mir_bb_ref_insert(ctx->deleted, cur);
      mir_bb_free(cur);
    } else {
      list_mir_bb_push_back(new_bbs, cur);
    }
  }

  list_mir_bb_free(sub->defined.bbs);
  sub->defined.bbs = new_bbs;

  hashset_mir_bb_ref_free(ctx->deleted);
  hashset_mir_bb_ref_free(ctx->visited);
  hashset_mir_bb_preds_free(ctx->preds);
}

mir_merge_bb_result mir_merge_bb(mir *mir) {
  mir_merge_bb_result result = {
      .exceptions = list_exception_new(),
  };

  mir_ctx ctx = {
      .preds      = NULL,
      .visited    = NULL,
      .deleted    = NULL,
      .exceptions = result.exceptions,
  };

  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->defined_subs);
       !END(it); NEXT(it)) {
    mir_subroutine *sub = GET(it);
    if (sub->kind == MIR_SUBROUTINE_DEFINED) {
      mir_ctx_bb_merge_subroutine(&ctx, sub);
    }
  }

  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->methods);
       !END(it); NEXT(it)) {
    mir_subroutine *sub = GET(it);
    if (sub->kind == MIR_SUBROUTINE_DEFINED) {
      mir_ctx_bb_merge_subroutine(&ctx, sub);
    }
  }

  return result;
}
