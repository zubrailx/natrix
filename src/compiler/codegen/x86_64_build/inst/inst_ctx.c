#include "inst.h"

#include "util/macro.h"
#include <stdio.h>
#include <string.h>

void cg_ctx_init(cg_ctx *ctx, cg_x86_64 *code, cg_debug *debug,
                 list_exception *exceptions) {
  ctx->map_mir_sym       = hashset_cg_mir_sym_new();
  ctx->lit_cnt           = 0;
  ctx->method_cnt        = 0;
  ctx->map_type_sym_init = hashset_cg_type_sym_new();
  ctx->class_cnt         = 0;

  ctx->sub            = NULL;
  ctx->sub_sym        = NULL;
  ctx->sub_debug      = NULL;
  ctx->line_cnt       = 0;
  ctx->map_value_meta = NULL;
  ctx->frame_size     = 0;

  ctx->bb = NULL;

  ctx->code       = code;
  ctx->debug      = debug;
  ctx->exceptions = exceptions;
}

void cg_ctx_deinit(cg_ctx *ctx) {
  hashset_cg_mir_sym_free(ctx->map_mir_sym);
  ctx->lit_cnt    = 0;
  ctx->method_cnt = 0;
  hashset_cg_type_sym_free(ctx->map_type_sym_init);
  ctx->class_cnt = 0;

  ctx->sub            = NULL;
  ctx->sub_sym        = NULL;
  ctx->sub_debug      = NULL;
  ctx->line_cnt       = 0;
  ctx->map_value_meta = NULL;
  ctx->frame_size     = 0;

  ctx->bb = NULL;

  ctx->code       = NULL;
  ctx->debug      = NULL;
  ctx->exceptions = NULL;
}

static inline const char *cg_ctx_mir_sym_find(cg_ctx     *ctx,
                                              const void *mir_ref) {
  hashset_cg_mir_sym_it it = hashset_cg_mir_sym_find(
      ctx->map_mir_sym, &(cg_mir_sym){.mir_ref = mir_ref});
  if (END(it)) {
    return NULL;
  }
  return GET(it)->sym_ref;
}

const char *cg_ctx_mir_sym_find_lit(cg_ctx *ctx, const mir_lit *lit_ref) {
  return cg_ctx_mir_sym_find(ctx, lit_ref);
}

char *cg_ctx_mir_sym_emplace_lit(cg_ctx *ctx, const mir_lit *lit_ref) {
  char buf[64];
  snprintf(buf, STRMAXLEN(buf), ".L_lit_%lu", ctx->lit_cnt++);

  char *sym = strdup(buf);

  hashset_cg_mir_sym_insert(ctx->map_mir_sym, cg_mir_sym_new(lit_ref, sym));

  return sym;
}

const char *cg_ctx_mir_sym_find_sub(cg_ctx               *ctx,
                                    const mir_subroutine *sub_ref) {
  return cg_ctx_mir_sym_find(ctx, sub_ref);
}

void cg_ctx_mir_sym_emplace_sub(cg_ctx *ctx, const mir_subroutine *sub_ref,
                                const char *sym_ref) {
  hashset_cg_mir_sym_insert(ctx->map_mir_sym, cg_mir_sym_new(sub_ref, sym_ref));
}

char *cg_ctx_mir_sym_emplace_method(cg_ctx               *ctx,
                                    const mir_subroutine *sub_ref) {
  char buf[64];
  snprintf(buf, STRMAXLEN(buf), ".L_method_%lu", ctx->method_cnt++);

  char *sym = strdup(buf);

  hashset_cg_mir_sym_insert(ctx->map_mir_sym, cg_mir_sym_new(sub_ref, sym));

  return sym;
}

const char *cg_ctx_type_sym_init_find_class(cg_ctx          *ctx,
                                            const type_mono *mono_ref) {
  hashset_cg_type_sym_it it = hashset_cg_type_sym_find(
      ctx->map_type_sym_init, &(cg_type_sym){.mono_ref = mono_ref});
  if (END(it)) {
    return NULL;
  }
  return GET(it)->sym_ref;
}

char *cg_ctx_type_sym_init_emplace_class(cg_ctx          *ctx,
                                         const type_mono *mono_ref) {
  char buf[64];
  snprintf(buf, STRMAXLEN(buf), ".L_class_init_%lu", ctx->class_cnt++);

  char *sym = strdup(buf);

  hashset_cg_type_sym_insert(ctx->map_type_sym_init,
                             cg_type_sym_new(mono_ref, sym));

  return sym;
}

cg_value_meta *cg_ctx_value_meta_find(cg_ctx *ctx, const mir_value *value) {
  hashset_cg_value_meta_it it = hashset_cg_value_meta_find(
      ctx->map_value_meta, &(cg_value_meta){.value_ref = value});
  if (END(it)) {
    return NULL;
  }
  return GET(it);
}

cg_value_meta *cg_ctx_value_meta_emplace(cg_ctx          *ctx,
                                         const mir_value *value_ref,
                                         int64_t offset, int is_ptr) {
  hashset_cg_value_meta_it it = hashset_cg_value_meta_insert(
      ctx->map_value_meta, cg_value_meta_new(value_ref, offset, is_ptr));
  return GET(it);
}

void cg_ctx_text_push_back(cg_ctx *ctx, void *unit) {
  list_cg_x86_64_unit_push_back(ctx->code->text, (cg_x86_64_unit *)unit);
}

void cg_ctx_data_push_back(cg_ctx *ctx, void *unit) {
  list_cg_x86_64_unit_push_back(ctx->code->data, (cg_x86_64_unit *)unit);
}

// args of type cg_x86_64_op*, last arg is NULL
uint64_t cg_ctx_text_emplace_back_text(cg_ctx *ctx, cg_x86_64_mnem mnem, ...) {
  va_list args;
  va_start(args, mnem);

  list_cg_x86_64_op *operands = list_cg_x86_64_op_new();

  cg_x86_64_op *op = NULL;
  while ((op = va_arg(args, cg_x86_64_op *))) {
    list_cg_x86_64_op_push_back(operands, op);
  }

  cg_x86_64_text *text = cg_x86_64_text_new(mnem, operands);
  cg_ctx_text_push_back(ctx, text);

  va_end(args);

  return cg_x86_64_mnem_size(mnem);
}

int64_t cg_ctx_rbp_offset(cg_ctx *ctx) {
  return (-ctx->frame_size) + CG_X86_64_SIZE_QUAD * 2;
}
