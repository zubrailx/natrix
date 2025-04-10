#include "compiler/codegen/exception.h"
#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/type_table/str.h"
#include "inst.h"
#include "util/log.h"
#include "util/macro.h"
#include "x86_64_core/value.h"
#include <string.h>

static void cg_inst_sub_def_prologue_push(cg_ctx *ctx) {
  list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.params);

  if (END(it)) {
    return;
  }
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RSI),
      NULL);
  cg_ctx_value_meta_emplace(ctx, GET(it), cg_ctx_rbp_offset(ctx), 1);

  if (!NEXT(it)) {
    return;
  }
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RDX),
      NULL);
  cg_ctx_value_meta_emplace(ctx, GET(it), cg_ctx_rbp_offset(ctx), 1);

  if (!NEXT(it)) {
    return;
  }
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RCX),
      NULL);
  cg_ctx_value_meta_emplace(ctx, GET(it), cg_ctx_rbp_offset(ctx), 1);

  if (!NEXT(it)) {
    return;
  }
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_R8),
      NULL);
  cg_ctx_value_meta_emplace(ctx, GET(it), cg_ctx_rbp_offset(ctx), 1);

  if (!NEXT(it)) {
    return;
  }
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_R9),
      NULL);
  cg_ctx_value_meta_emplace(ctx, GET(it), cg_ctx_rbp_offset(ctx), 1);

  // first pushed last (top), last pushed first
  int64_t param_rbp_offset = CG_X86_64_SIZE_QUAD * 2;
  while (NEXT(it)) {
    ctx->frame_size += cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_PUSHQ,
        cg_x86_64_op_new_base_imm(param_rbp_offset, CG_X86_64_REG_RBP), NULL);
    cg_ctx_value_meta_emplace(ctx, GET(it), cg_ctx_rbp_offset(ctx), 1);

    param_rbp_offset += CG_X86_64_SIZE_QUAD;
  }
}

// returns additionally reserved frame size
static void cg_inst_sub_def_prologue(cg_ctx *ctx) {
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RBP),
      NULL);

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_MOVQ, cg_x86_64_op_new_register(CG_X86_64_REG_RSP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RBP), NULL);

  // return value - passed as first argument, copy reference to stack
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RDI),
      NULL);
  cg_ctx_value_meta_emplace(ctx, ctx->sub->defined.ret, cg_ctx_rbp_offset(ctx),
                            1);

  cg_inst_sub_def_prologue_push(ctx);
}

// allocate local memory for all params and copy them
static void cg_inst_sub_def_params_copy(cg_ctx *ctx) {
  uint64_t params           = list_mir_value_size(ctx->sub->defined.params);
  uint64_t frame_size_old   = ctx->frame_size;
  uint64_t frame_size_start = ctx->frame_size - params * CG_X86_64_SIZE_QUAD;

  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_text(cg_sym_local_suf(ctx->sub_sym, "copy")));

  // update stack pointer, because of call
  ctx->frame_size = frame_size_start + sizeof(x86_64_value) * params;
  ctx->frame_size = cg_aligned(ctx->frame_size);

  if (frame_size_old != ctx->frame_size) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_SUBQ,
        cg_x86_64_op_new_immediate(ctx->frame_size - frame_size_old),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
  }

  // reverse
  list_mir_value_ref *params_rv = list_mir_value_ref_new();
  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.params);
       !END(it); NEXT(it)) {
    list_mir_value_ref_push_front(params_rv, GET(it));
  }

  uint64_t offset_new = -(frame_size_start + CG_X86_64_SIZE_QUAD +
                          (sizeof(x86_64_value) * (params - 1)));

  for (list_mir_value_ref_it it = list_mir_value_ref_begin(params_rv); !END(it);
       NEXT(it), offset_new += sizeof(x86_64_value)) {
    const mir_value *value      = GET(it);
    cg_value_meta   *value_meta = cg_ctx_value_meta_find(ctx, value);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(offset_new, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(value_meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(offsetof(x86_64_value, op_tbl),
                                  CG_X86_64_REG_RSI),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(offsetof(x86_64_op_tbl, op_copy),
                                  CG_X86_64_REG_RAX),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                  cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                                  NULL);

    value_meta->is_ptr = 0;
    value_meta->offset = offset_new;
  }

  list_mir_value_ref_free(params_rv);
}

static void cg_inst_sub_def_epilogue(cg_ctx *ctx) {
  cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(
                                 cg_sym_local_suf(ctx->sub_sym, "epilogue")));

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_MOVQ, cg_x86_64_op_new_register(CG_X86_64_REG_RBP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);

  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_POPQ,
                                cg_x86_64_op_new_register(CG_X86_64_REG_RBP),
                                NULL);

  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_RETQ, NULL);
}

static void cg_inst_sub_def_locals_init(cg_ctx *ctx, int ret) {
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_text(cg_sym_local_suf(ctx->sub_sym, "init")));

  uint64_t frame_size_old = ctx->frame_size;

  // prepare local variables
  if (ret) {
    ctx->frame_size += sizeof(x86_64_value);
    cg_ctx_value_meta_emplace(ctx, ctx->sub->defined.ret,
                              cg_ctx_rbp_offset(ctx), 0);
  }

  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.vars);
       !END(it); NEXT(it)) {
    const mir_value *value = GET(it);
    ctx->frame_size += sizeof(x86_64_value);
    cg_ctx_value_meta_emplace(ctx, value, cg_ctx_rbp_offset(ctx), 0);
  }
  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.tmps);
       !END(it); NEXT(it)) {
    const mir_value *value = GET(it);
    ctx->frame_size += sizeof(x86_64_value);
    cg_ctx_value_meta_emplace(ctx, value, cg_ctx_rbp_offset(ctx), 0);
  }

  // allocate memory on  stack
  ctx->frame_size = cg_aligned(ctx->frame_size);
  if (frame_size_old != ctx->frame_size) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_SUBQ,
        cg_x86_64_op_new_immediate(ctx->frame_size - frame_size_old),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
  }

  // initialize
  if (ret) {
    const mir_value     *value = ctx->sub->defined.ret;
    const cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_make_void")), NULL);
  }

  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.vars);
       !END(it); NEXT(it)) {
    const mir_value     *value = GET(it);
    const cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_make_void")), NULL);
  }

  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.tmps);
       !END(it); NEXT(it)) {
    const mir_value     *value = GET(it);
    const cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_make_void")), NULL);
  }
}

static void cg_inst_sub_def_locals_deinit(cg_ctx *ctx) {
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_text(cg_sym_local_suf(ctx->sub_sym, "deinit")));

  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.vars);
       !END(it); NEXT(it)) {
    const mir_value     *value = GET(it);
    const cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(meta->offset + offsetof(x86_64_value, op_tbl),
                                  CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(offsetof(x86_64_op_tbl, op_drop),
                                  CG_X86_64_REG_RAX),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                  cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                                  NULL);
  }

  for (list_mir_value_it it = list_mir_value_begin(ctx->sub->defined.tmps);
       !END(it); NEXT(it)) {
    const mir_value     *value = GET(it);
    const cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(meta->offset + offsetof(x86_64_value, op_tbl),
                                  CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(offsetof(x86_64_op_tbl, op_drop),
                                  CG_X86_64_REG_RAX),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                  cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                                  NULL);
  }
}

static int cg_inst_sub_def_ok(cg_ctx *ctx, const mir_subroutine *sub) {
  const char *sub_name = sub->symbol_ref->name;

  if (sub->kind != MIR_SUBROUTINE_DEFINED) {
    error("subroutine '%s' def got kind %d %p, expected %d", sub_name,
          sub->kind, sub, MIR_SUBROUTINE_DEFINED);
    return 0;
  }

  if (sub->spec & MIR_SUBROUTINE_SPEC_EXTERN) {
    cg_exception_add_error(
        ctx->exceptions, EXCEPTION_CG_UNEXPECTED_EXTERN_DEFINITION,
        sub->symbol_ref->span,
        "subroutine '%s' can't be instantiated: is defined, but extern",
        sub_name);
    return 0;
  }

  return 1;
}

void cg_inst_sub_def(cg_ctx *ctx, const mir_subroutine *sub, char *sub_sym) {
  if (!cg_inst_sub_def_ok(ctx, sub)) {
    free(sub_sym);
    return;
  }

  ctx->sub      = sub;
  ctx->sub_sym  = sub_sym;
  ctx->line_cnt = 0;

  if (cg_debug_enabled(ctx->debug)) {
    ctx->sub_debug = cg_debug_sub_new(NULL, NULL, sub->symbol_ref->name);
  }

  cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sub_sym));

  ctx->map_value_meta = hashset_cg_value_meta_new();
  ctx->frame_size     = CG_X86_64_SIZE_QUAD;

  if (cg_debug_enabled(ctx->debug)) {
    char *sym_start = cg_sym_local_suf(ctx->sub_sym, "S");
    cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sym_start));
    ctx->sub_debug->sym_start_ref = sym_start;
  }

  cg_inst_sub_def_prologue(ctx);

  cg_inst_sub_def_params_copy(ctx);

  cg_inst_sub_def_locals_init(ctx, 0);

  // debug("locals_value_meta '%s':", ctx->sub_sym);
  // for (hashset_cg_value_meta_it it =
  //          hashset_cg_value_meta_begin(ctx->map_value_meta);
  //      !END(it); NEXT(it)) {
  //   cg_value_meta *meta = GET(it);
  //   debug("  _%lu -> p=%d, off=%ld(-0x%lx)", meta->value_ref->id,
  //   meta->is_ptr,
  //         meta->offset, -meta->offset);
  // }

  if (cg_debug_enabled(ctx->debug)) {
    for (list_mir_value_it it = list_mir_value_begin(sub->defined.params);
         !END(it); NEXT(it)) {
      mir_value     *value = GET(it);
      cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);
      list_cg_debug_param_push_back(
          ctx->sub_debug->params,
          cg_debug_param_new(meta->offset, value->symbol_ref->name));
    }

    for (list_mir_value_it it = list_mir_value_begin(sub->defined.vars);
         !END(it); NEXT(it)) {
      mir_value     *value = GET(it);
      cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);
      list_cg_debug_var_push_back(
          ctx->sub_debug->vars,
          cg_debug_var_new(meta->offset, value->symbol_ref->name));
    }
  }

  cg_inst_bbs(ctx, sub->defined.bbs);

  cg_inst_sub_def_locals_deinit(ctx);

  cg_inst_sub_def_epilogue(ctx);

  if (cg_debug_enabled(ctx->debug)) {
    char *sym_end = cg_sym_local_suf(ctx->sub_sym, "E");
    cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sym_end));
    ctx->sub_debug->sym_end_ref = sym_end;

    list_cg_debug_sub_push_back(ctx->debug->subroutines, ctx->sub_debug);
  }

  hashset_cg_value_meta_free(ctx->map_value_meta);
}

static void cg_inst_sub_main_prologue(cg_ctx *ctx) {
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RBP),
      NULL);

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_MOVQ, cg_x86_64_op_new_register(CG_X86_64_REG_RSP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RBP), NULL);
}

static void cg_inst_sub_main_epilogue(cg_ctx *ctx) {
  cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(
                                 cg_sym_local_suf(ctx->sub_sym, "epilogue")));

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_CALL,
      cg_x86_64_op_new_direct(strdup("__x86_64_flush")), NULL);

  const type_base *ret_type = ctx->sub->defined.ret->type_ref->type;

  if (ret_type && ret_type->kind == TYPE_PRIMITIVE &&
      ((type_primitive *)ret_type)->type == TYPE_PRIMITIVE_INT) {

    cg_value_meta *ret_meta =
        cg_ctx_value_meta_find(ctx, ctx->sub->defined.ret);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(ret_meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_int")), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVL, cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);
  } else {
    cg_exception_add_error(ctx->exceptions, EXCEPTION_CG_UNEXPECTED_RETURN_TYPE,
                           ctx->sub->symbol_ref->span,
                           "expected subroutine '%s' return type int",
                           ctx->sub->symbol_ref->name);
  }

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_MOVQ, cg_x86_64_op_new_immediate(60),
      cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_SYSCALL, NULL);
}

void cg_inst_sub_main(cg_ctx *ctx, const mir_subroutine *sub, char *sub_sym) {
  if (!cg_inst_sub_def_ok(ctx, sub)) {
    free(sub_sym);
    return;
  }

  ctx->sub      = sub;
  ctx->sub_sym  = sub_sym;
  ctx->line_cnt = 0;

  if (cg_debug_enabled(ctx->debug)) {
    ctx->sub_debug = cg_debug_sub_new(NULL, NULL, sub->symbol_ref->name);
  }

  cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sub_sym));

  ctx->map_value_meta = hashset_cg_value_meta_new();
  ctx->frame_size     = CG_X86_64_SIZE_QUAD;

  if (cg_debug_enabled(ctx->debug)) {
    char *sym_start = cg_sym_local_suf(ctx->sub_sym, "S");
    cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sym_start));
    ctx->sub_debug->sym_start_ref = sym_start;
  }

  cg_inst_sub_main_prologue(ctx);

  cg_inst_sub_def_locals_init(ctx, 1);

  // debug("locals_value_meta '%s':", ctx->sub_sym);
  // for (hashset_cg_value_meta_it it =
  //          hashset_cg_value_meta_begin(ctx->map_value_meta);
  //      !END(it); NEXT(it)) {
  //   cg_value_meta *meta = GET(it);
  //   debug("  _%lu -> p=%d, off=%ld(-0x%lx)", meta->value_ref->id,
  //   meta->is_ptr,
  //         meta->offset, -meta->offset);
  // }

  if (cg_debug_enabled(ctx->debug)) {
    for (list_mir_value_it it = list_mir_value_begin(sub->defined.params);
         !END(it); NEXT(it)) {
      mir_value     *value = GET(it);
      cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);
      list_cg_debug_param_push_back(
          ctx->sub_debug->params,
          cg_debug_param_new(meta->offset, value->symbol_ref->name));
    }

    for (list_mir_value_it it = list_mir_value_begin(sub->defined.vars);
         !END(it); NEXT(it)) {
      mir_value     *value = GET(it);
      cg_value_meta *meta  = cg_ctx_value_meta_find(ctx, value);
      list_cg_debug_var_push_back(
          ctx->sub_debug->vars,
          cg_debug_var_new(meta->offset, value->symbol_ref->name));
    }
  }

  cg_inst_bbs(ctx, sub->defined.bbs);

  cg_inst_sub_def_locals_deinit(ctx);

  cg_inst_sub_main_epilogue(ctx);

  if (cg_debug_enabled(ctx->debug)) {
    char *sym_end = cg_sym_local_suf(ctx->sub_sym, "E");
    cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sym_end));
    ctx->sub_debug->sym_end_ref = sym_end;

    list_cg_debug_sub_push_back(ctx->debug->subroutines, ctx->sub_debug);
  }

  hashset_cg_value_meta_free(ctx->map_value_meta);
}

int cg_inst_sub_extern_check_param(const type_base *param) {
  int param_ok = 1;

  switch (param->kind) {
    case TYPE_PRIMITIVE: {
      const type_primitive *prim = (typeof(prim))param;
      switch (prim->type) {
        case TYPE_PRIMITIVE_BOOL:
        case TYPE_PRIMITIVE_BYTE:
        case TYPE_PRIMITIVE_INT:
        case TYPE_PRIMITIVE_UINT:
        case TYPE_PRIMITIVE_LONG:
        case TYPE_PRIMITIVE_ULONG:
        case TYPE_PRIMITIVE_CHAR:
        case TYPE_PRIMITIVE_STRING:
          break;
        case TYPE_PRIMITIVE_VOID:
        case TYPE_PRIMITIVE_ANY:
        default:
          param_ok = 0;
          break;
      }
      break;
    }
    case TYPE_ARRAY:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME:
    case TYPE_MONO:
      param_ok = 0;
      break;
  }
  return param_ok;
}

int cg_inst_sub_extern_check_ret(const type_base *ret) {
  int ret_ok = 1;

  switch (ret->kind) {
    case TYPE_PRIMITIVE: {
      const type_primitive *prim = (typeof(prim))ret;
      switch (prim->type) {
        case TYPE_PRIMITIVE_BOOL:
        case TYPE_PRIMITIVE_BYTE:
        case TYPE_PRIMITIVE_INT:
        case TYPE_PRIMITIVE_UINT:
        case TYPE_PRIMITIVE_LONG:
        case TYPE_PRIMITIVE_ULONG:
        case TYPE_PRIMITIVE_CHAR:
        case TYPE_PRIMITIVE_STRING:
        case TYPE_PRIMITIVE_VOID:
          break;
        case TYPE_PRIMITIVE_ANY:
        default:
          ret_ok = 0;
          break;
      }
      break;
    }
    case TYPE_ARRAY:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME:
    case TYPE_MONO:
      ret_ok = 0;
      break;
  }
  return ret_ok;
}

int cg_inst_sub_decl_check(cg_ctx *ctx, const mir_subroutine *sub) {
  const char *sub_sym = cg_ctx_mir_sym_find_sub(ctx, sub);
  if (!sub_sym) {
    error("subroutine not found in mir index");
    return 0;
  }

  union {
    const type_base     *base;
    const type_callable *sub;
  } type;

  type.base = sub->type_ref->type;

  if (type.base->kind != TYPE_CALLABLE) {
    error("subroutine %s expected callable type for subroutine", sub_sym);
    return 0;
  }

  int ok = 1;

  if (!(sub->spec & MIR_SUBROUTINE_SPEC_EXTERN)) {
    return ok;
  }

  for (list_type_ref_it it = list_type_ref_begin(type.sub->params); !END(it);
       NEXT(it)) {
    const type_base *param    = GET(it);
    int              param_ok = cg_inst_sub_extern_check_param(param);

    if (!param_ok) {
      char *type_s = type_str(param);
      cg_exception_add_error(
          ctx->exceptions, EXCEPTION_CG_UNSUPPORTED_EXTERN_TYPE,
          param->type_entry_ref->span,
          "subroutine '%s' param type '%s' is unsupported", sub_sym, type_s);
      free(type_s);
      ok = 0;
    }
  }

  const type_base *ret    = type.sub->ret_ref;
  int              ret_ok = cg_inst_sub_extern_check_ret(ret);

  if (!ret_ok) {
    char *type_s = type_str(ret);
    cg_exception_add_error(
        ctx->exceptions, EXCEPTION_CG_UNSUPPORTED_EXTERN_TYPE,
        ret->type_entry_ref->span,
        "subroutine '%s' ret type '%s' is unsupported", sub_sym, type_s);
    free(type_s);
    ok = 0;
  }
  return ok;
}
