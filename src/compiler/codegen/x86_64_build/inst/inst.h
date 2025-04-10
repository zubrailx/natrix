#pragma once

#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/codegen/x86_64_build/data.h"
#include "compiler/codegen/x86_64_build/debug.h"
#include "compiler/exception/list.h"
#include "compiler/mir/mir.h"

typedef struct cg_inst_result_struct {
  cg_debug       *debug;
  list_exception *exceptions;
} cg_inst_result;

cg_inst_result cg_inst(cg_x86_64 *code, const mir *mir);

// ctx
typedef struct cg_ctx_struct {
  hashset_cg_mir_sym  *map_mir_sym;
  uint64_t             lit_cnt;
  uint64_t             method_cnt;
  hashset_cg_type_sym *map_type_sym_init; // initializer funcs
  uint64_t             class_cnt;

  const mir_subroutine  *sub;
  const char            *sub_sym;
  cg_debug_sub          *sub_debug;
  uint64_t               line_cnt; // also for debug
  hashset_cg_value_meta *map_value_meta;
  uint64_t               frame_size;

  const mir_bb *bb;

  cg_debug       *debug;
  cg_x86_64      *code;
  list_exception *exceptions;
} cg_ctx;

void cg_ctx_init(cg_ctx *ctx, cg_x86_64 *code, cg_debug *debug,
                 list_exception *exceptions);
void cg_ctx_deinit(cg_ctx *ctx);

const char *cg_ctx_mir_sym_find_lit(cg_ctx *ctx, const mir_lit *lit_ref);
char       *cg_ctx_mir_sym_emplace_lit(cg_ctx *ctx, const mir_lit *lit_ref);

const char *cg_ctx_mir_sym_find_sub(cg_ctx *ctx, const mir_subroutine *sub_ref);
void cg_ctx_mir_sym_emplace_sub(cg_ctx *ctx, const mir_subroutine *sub_ref,
                                const char *sym_ref);

char *cg_ctx_mir_sym_emplace_method(cg_ctx *ctx, const mir_subroutine *sub_ref);

const char *cg_ctx_type_sym_init_find_class(cg_ctx          *ctx,
                                            const type_mono *mono_ref);
char       *cg_ctx_type_sym_init_emplace_class(cg_ctx          *ctx,
                                               const type_mono *mono_ref);

cg_value_meta *cg_ctx_value_meta_find(cg_ctx *ctx, const mir_value *value);
cg_value_meta *cg_ctx_value_meta_emplace(cg_ctx          *ctx,
                                         const mir_value *value_ref,
                                         int64_t offset, int is_ptr);

void cg_ctx_text_push_back(cg_ctx *ctx, void *unit);
void cg_ctx_data_push_back(cg_ctx *ctx, void *unit);
void cg_ctx_debug_info_push_back(cg_ctx *ctx, void *unit);
void cg_ctx_debug_line_push_back(cg_ctx *ctx, void *unit);
void cg_ctx_debug_str_push_back(cg_ctx *ctx, void *unit);

uint64_t cg_ctx_text_emplace_back_text(cg_ctx *ctx, cg_x86_64_mnem mnem, ...);

int64_t cg_ctx_rbp_offset(cg_ctx *ctx);

// sub
int  cg_inst_sub_extern_check_param(const type_base *param);
int  cg_inst_sub_extern_check_ret(const type_base *ret);
int  cg_inst_sub_decl_check(cg_ctx *ctx, const mir_subroutine *sub);
void cg_inst_sub_def(cg_ctx *ctx, const mir_subroutine *sub, char *sub_sym);
void cg_inst_sub_main(cg_ctx *ctx, const mir_subroutine *sub, char *sub_sym);

// labels
char *cg_sym_local_suf(const char *base, const char *suffix);
char *cg_sym_local_suf_idx(const char *base, const char *suffix, uint64_t idx);
char *cg_sym_local_bb(const char *base, const mir_bb *bb);

// utility
uint64_t cg_aligned(uint64_t size);

// bb
void cg_inst_bbs(cg_ctx *ctx, const list_mir_bb *bbs);

// core
void cg_inst_core(cg_ctx *ctx);

// class
void cg_inst_class_methods(cg_ctx *ctx, list_cg_method_sym *methods);
void cg_inst_class_inits(cg_ctx *ctx, list_cg_class_sym *class_inits);
