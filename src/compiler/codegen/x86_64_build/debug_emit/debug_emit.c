#include "debug_emit.h"

#include "util/hashset.h"
#include "util/list.h"
#include "util/macro.h"
#include <string.h>

typedef struct cg_debug_chars_struct {
  uint64_t    idx;
  const char *chars;
} cg_debug_chars;

static cg_debug_chars *cg_debug_chars_new(uint64_t idx, const char *chars) {
  cg_debug_chars *self = MALLOC(cg_debug_chars);
  self->idx            = idx;
  self->chars          = chars;
  return self;
}

static void cg_debug_chars_free(cg_debug_chars *self) {
  if (self) {
    free(self);
  }
}

static inline void container_delete_cg_debug_chars(void *data) {
  cg_debug_chars_free(data);
}
static inline int container_cmp_cg_debug_chars(const void *lsv,
                                               const void *rsv) {
  const cg_debug_chars *l = lsv;
  const cg_debug_chars *r = rsv;
  return container_cmp_chars(l->chars, r->chars);
}
static inline uint64_t container_hash_cg_debug_chars(const void *lsv) {
  const cg_debug_chars *l = lsv;
  return container_hash_chars(l->chars);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_cg_debug_chars, cg_debug_chars,
                              container_cmp_cg_debug_chars, container_new_move,
                              container_delete_cg_debug_chars,
                              container_hash_cg_debug_chars);

LIST_DECLARE_STATIC_INLINE(list_cg_debug_chars_ref, cg_debug_chars,
                           container_cmp_false, container_new_move,
                           container_delete_false);

static inline cg_debug_chars *
cg_debug_chars_find(hashset_cg_debug_chars *hashset, const char *chars) {
  hashset_cg_debug_chars_it it =
      hashset_cg_debug_chars_find(hashset, &(cg_debug_chars){.chars = chars});
  if (!END(it)) {
    return GET(it);
  }
  return NULL;
}

static inline cg_debug_chars *
cg_debug_chars_emplace(hashset_cg_debug_chars *hashset, uint64_t idx,
                       const char *chars) {
  cg_debug_chars           *debug = cg_debug_chars_new(idx, chars);
  hashset_cg_debug_chars_it it = hashset_cg_debug_chars_insert(hashset, debug);
  return GET(it);
}

typedef struct cg_ctx_struct {
  hashset_cg_debug_chars  *files;
  uint64_t                 files_cnt;
  list_cg_debug_chars_ref *file_refs; // to preserve order
  hashset_cg_debug_chars  *strs;
  uint64_t                 strs_cnt;
  list_cg_debug_chars_ref *str_refs; // to preserve order

  cg_x86_64 *code;
} cg_ctx;

static void cg_ctx_init(cg_ctx *ctx, cg_x86_64 *code) {
  ctx->files     = hashset_cg_debug_chars_new();
  ctx->files_cnt = 0;
  ctx->file_refs = list_cg_debug_chars_ref_new();
  ctx->strs      = hashset_cg_debug_chars_new();
  ctx->strs_cnt  = 0;
  ctx->str_refs  = list_cg_debug_chars_ref_new();

  ctx->code = code;
}

static void cg_ctx_deinit(cg_ctx *ctx) {
  hashset_cg_debug_chars_free(ctx->files);
  ctx->files_cnt = 0;
  list_cg_debug_chars_ref_free(ctx->file_refs);
  hashset_cg_debug_chars_free(ctx->strs);
  ctx->strs_cnt = 0;
  list_cg_debug_chars_ref_free(ctx->str_refs);

  ctx->code = NULL;
}

static void cg_ctx_info_push(cg_ctx *ctx, void *unit) {
  list_cg_x86_64_unit_push_back(ctx->code->debug_info, unit);
}

static void cg_ctx_line_push(cg_ctx *ctx, void *unit) {
  list_cg_x86_64_unit_push_back(ctx->code->debug_line, unit);
}

static void cg_ctx_str_push(cg_ctx *ctx, void *unit) {
  list_cg_x86_64_unit_push_back(ctx->code->debug_str, unit);
}

cg_debug_emit_result cg_debug_emit(cg_x86_64 *code, const cg_debug *debug) {
  cg_debug_emit_result result = {
      .exceptions = list_exception_new(),
  };

  cg_ctx ctx;
  cg_ctx_init(&ctx, code);

  // DEBUG_LINE
  cg_ctx_line_push(
      &ctx, cg_x86_64_data_new_quad(list_cg_debug_line_size(debug->lines)));

  // lines
  for (list_cg_debug_line_it it = list_cg_debug_line_begin(debug->lines);
       !END(it); NEXT(it)) {
    const cg_debug_line *line = GET(it);

    const cg_debug_chars *file;
    const cg_debug_chars *str;

    // add file unique to list (for ordering) and hashset (for indexing)
    if (!(file = cg_debug_chars_find(ctx.files, line->file_ref))) {
      uint64_t file_id = ctx.files_cnt++;
      file = cg_debug_chars_emplace(ctx.files, file_id, line->file_ref);
      list_cg_debug_chars_ref_push_back(ctx.file_refs, (cg_debug_chars *)file);
    }

    if (!cg_debug_chars_find(ctx.strs, line->file_ref)) {
      str = cg_debug_chars_emplace(ctx.strs, ctx.strs_cnt++, line->file_ref);
      list_cg_debug_chars_ref_push_back(ctx.str_refs, (cg_debug_chars *)str);
    }

    cg_ctx_line_push(&ctx,
                     cg_x86_64_data_new_symbol(strdup(line->sym_address_ref)));
    cg_ctx_line_push(&ctx, cg_x86_64_data_new_quad(file->idx));
    cg_ctx_line_push(&ctx, cg_x86_64_data_new_quad(line->line));
  }

  // DEBUG_INFO
  cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(
                             list_cg_debug_sub_size(debug->subroutines)));
  cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(ctx.files_cnt));

  // subroutines
  for (list_cg_debug_sub_it it = list_cg_debug_sub_begin(debug->subroutines);
       !END(it); NEXT(it)) {
    const cg_debug_sub   *s   = GET(it);
    const cg_debug_chars *s_s = NULL;

    // address_start + address_end
    cg_ctx_info_push(&ctx, cg_x86_64_data_new_symbol(strdup(s->sym_start_ref)));
    cg_ctx_info_push(&ctx, cg_x86_64_data_new_symbol(strdup(s->sym_end_ref)));

    if (!(s_s = cg_debug_chars_find(ctx.strs, s->name_ref))) {
      s_s = cg_debug_chars_emplace(ctx.strs, ctx.strs_cnt++, s->name_ref);
      list_cg_debug_chars_ref_push_back(ctx.str_refs, (cg_debug_chars *)s_s);
    }

    // debug_str_id(name) + params_cnt + vars_cnt
    cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(s_s->idx));
    cg_ctx_info_push(
        &ctx, cg_x86_64_data_new_quad(list_cg_debug_param_size(s->params)));
    cg_ctx_info_push(&ctx,
                     cg_x86_64_data_new_quad(list_cg_debug_var_size(s->vars)));

    // param: rbp_offset + debug_str_id(name)
    for (list_cg_debug_param_it it_p = list_cg_debug_param_begin(s->params);
         !END(it_p); NEXT(it_p)) {
      const cg_debug_param *p   = GET(it_p);
      const cg_debug_chars *p_s = NULL;

      cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(p->rbp_offset));

      if (!(p_s = cg_debug_chars_find(ctx.strs, p->name_ref))) {
        p_s = cg_debug_chars_emplace(ctx.strs, ctx.strs_cnt++, p->name_ref);
        list_cg_debug_chars_ref_push_back(ctx.str_refs, (cg_debug_chars *)p_s);
      }

      cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(p_s->idx));
    }

    // var: rbp_offset + debug_str_id(name)
    for (list_cg_debug_var_it it_v = list_cg_debug_var_begin(s->vars);
         !END(it_v); NEXT(it_v)) {
      const cg_debug_var   *v   = GET(it_v);
      const cg_debug_chars *v_s = NULL;

      cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(v->rbp_offset));

      if (!(v_s = cg_debug_chars_find(ctx.strs, v->name_ref))) {
        v_s = cg_debug_chars_emplace(ctx.strs, ctx.strs_cnt++, v->name_ref);
        list_cg_debug_chars_ref_push_back(ctx.str_refs, (cg_debug_chars *)v_s);
      }

      cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(v_s->idx));
    }
  }

  // files
  for (list_cg_debug_chars_ref_it it =
           list_cg_debug_chars_ref_begin(ctx.file_refs);
       !END(it); NEXT(it)) {
    const cg_debug_chars *file = GET(it);
    cg_ctx_info_push(&ctx, cg_x86_64_data_new_quad(file->idx));
  }

  // DEBUG_STR
  cg_ctx_str_push(&ctx, cg_x86_64_data_new_quad(ctx.strs_cnt));

  for (list_cg_debug_chars_ref_it it =
           list_cg_debug_chars_ref_begin(ctx.str_refs);
       !END(it); NEXT(it)) {
    const cg_debug_chars *str = GET(it);
    cg_ctx_str_push(&ctx,
                    cg_x86_64_data_new_ascii((uint8_t *)strdup(str->chars)));
  }

  cg_ctx_deinit(&ctx);

  return result;
}
