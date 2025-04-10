#include "compiler/codegen/exception.h"
#include "inst.h"

#include "util/log.h"
#include "util/macro.h"
#include "x86_64_core/value.h"
#include <string.h>

void cg_inst_class_methods(cg_ctx *ctx, list_cg_method_sym *methods) {
  UNUSED(ctx);
  while (!list_cg_method_sym_empty(methods)) {
    cg_method_sym        *method_sym = list_cg_method_sym_pop_front(methods);
    const mir_subroutine *sub        = method_sym->method_ref;

    switch (sub->kind) {
      case MIR_SUBROUTINE_DEFINED:
        cg_inst_sub_def(ctx, sub, method_sym->sym);
        method_sym->sym = NULL;
        break;
      case MIR_SUBROUTINE_DECLARED:
        cg_exception_add_error(ctx->exceptions,
                               EXCEPTION_CG_UNSUPPORTED_CLASS_DECLARED,
                               sub->symbol_ref->span,
                               "TODO: subroutine '%s' can't be instantiated",
                               sub->symbol_ref->name);
        break;
      case MIR_SUBROUTINE_IMPORTED:
        cg_exception_add_error(ctx->exceptions,
                               EXCEPTION_CG_UNSUPPORTED_CLASS_IMPORTED,
                               sub->symbol_ref->span,
                               "TODO: subroutine '%s' can't be instantiated",
                               sub->symbol_ref->name);
        break;
    }

    cg_method_sym_free(method_sym);
  }

  list_cg_method_sym_free(methods);
}

static const char *cg_inst_class_init_symbols(cg_ctx *ctx,
                                              const mir_class *class) {

  // add member names outside of symbols (to then reference them)
  uint64_t cnt = 0;
  for (list_mir_value_it it = list_mir_value_begin(class->fields); !END(it);
       NEXT(it), ++cnt) {
    const mir_value *value      = GET(it);
    const char      *value_name = value->symbol_ref->name;

    char *sym = cg_sym_local_suf_idx(ctx->sub_sym, "name", cnt);

    cg_ctx_data_push_back(ctx, cg_x86_64_symbol_new_data(sym));
    cg_ctx_data_push_back(
        ctx, cg_x86_64_data_new_ascii((uint8_t *)strdup(value_name)));
  }

  for (list_mir_subroutine_ref_it it =
           list_mir_subroutine_ref_begin(class->methods);
       !END(it); NEXT(it), ++cnt) {
    const mir_subroutine *value      = GET(it);
    const char           *value_name = value->symbol_ref->name;

    char *sym = cg_sym_local_suf_idx(ctx->sub_sym, "name", cnt);

    cg_ctx_data_push_back(ctx, cg_x86_64_symbol_new_data(sym));
    cg_ctx_data_push_back(
        ctx, cg_x86_64_data_new_ascii((uint8_t *)strdup(value_name)));
  }

  // construct
  const char *symbols;

  {
    char *sym = cg_sym_local_suf(ctx->sub_sym, "symbols");
    cg_ctx_data_push_back(ctx, cg_x86_64_symbol_new_data_ln(sym));
    symbols = sym;
  }

  cg_ctx_data_push_back(ctx, cg_x86_64_data_new_quad(cnt));

  for (uint64_t i = 0; i < cnt; ++i) {
    char *sym = cg_sym_local_suf_idx(ctx->sub_sym, "name", i);
    cg_ctx_data_push_back(ctx, cg_x86_64_data_new_symbol(sym));
  }

  return symbols;
}

static void cg_inst_class_init_defaults(cg_ctx *ctx, const mir_class *class) {
  // count entries
  uint64_t cnt = 0;
  for (list_mir_value_it it = list_mir_value_begin(class->fields); !END(it);
       NEXT(it)) {
    ++cnt;
  }
  for (list_mir_subroutine_ref_it it =
           list_mir_subroutine_ref_begin(class->methods);
       !END(it); NEXT(it)) {
    ++cnt;
  }

  // reserve stack required for defaults with alignment
  uint64_t frame_size_new =
      cg_aligned(ctx->frame_size + cnt * sizeof(x86_64_value));

  if (frame_size_new != ctx->frame_size) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_SUBQ,
        cg_x86_64_op_new_immediate(frame_size_new - ctx->frame_size),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
    ctx->frame_size = frame_size_new;
  }

  uint64_t idx = 0;

  // initialize values to void
  for (list_mir_value_it it = list_mir_value_begin(class->fields); !END(it);
       NEXT(it), ++idx) {

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(idx * sizeof(x86_64_value),
                                  CG_X86_64_REG_RSP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_make_void")), NULL);
  }

  // initialize methods
  for (list_mir_subroutine_ref_it it =
           list_mir_subroutine_ref_begin(class->methods);
       !END(it); NEXT(it), ++idx) {
    const mir_subroutine *sub     = GET(it);
    const char           *sub_sym = cg_ctx_mir_sym_find_sub(ctx, sub);

    if (!sub_sym) {
      error("method '%s' not found in symbol index", sub->symbol_ref->name);
      continue;
    }

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_imm(idx * sizeof(x86_64_value),
                                  CG_X86_64_REG_RSP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_LEAQ,
        cg_x86_64_op_new_base_sym(strdup(sub_sym), CG_X86_64_REG_RIP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_make_callable")), NULL);
  }
}

static void cg_inst_class_init(cg_ctx *ctx, const mir_class *class,
                               char   *init_sym) {
  ctx->sub     = NULL;
  ctx->sub_sym = init_sym;

  cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(init_sym));

  ctx->frame_size         = CG_X86_64_SIZE_QUAD;
  uint64_t frame_size_old = ctx->frame_size;

  // store %rdi, it will be overwritten by constructing defaults
  ctx->frame_size += cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_register(CG_X86_64_REG_RDI),
      NULL);

  const char *init_symbols_sym = cg_inst_class_init_symbols(ctx, class);
  cg_inst_class_init_defaults(ctx, class);

  // restore %rdi from stack
  uint64_t rsp_rdi_offset =
      ctx->frame_size - (frame_size_old + CG_X86_64_SIZE_QUAD);
  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_MOVQ,
      cg_x86_64_op_new_base_imm(rsp_rdi_offset, CG_X86_64_REG_RSP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);

  // construct symbols table and store pointer to symbol table in %rsi
  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_LEAQ,
      cg_x86_64_op_new_base_sym(strdup(init_symbols_sym), CG_X86_64_REG_RIP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

  // set %rdx to current stack pointer (where defaults are created)
  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_LEAQ, cg_x86_64_op_new_base_imm(0, CG_X86_64_REG_RSP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_CALL,
      cg_x86_64_op_new_direct(strdup("__x86_64_make_object_setup")), NULL);

  // restore stack
  if (frame_size_old != ctx->frame_size) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_ADDQ,
        cg_x86_64_op_new_immediate(ctx->frame_size - frame_size_old),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
  }

  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_RETQ, NULL);
}

void cg_inst_class_inits(cg_ctx *ctx, list_cg_class_sym *class_inits) {
  UNUSED(ctx);

  while (!list_cg_class_sym_empty(class_inits)) {
    cg_class_sym *class_sym = list_cg_class_sym_pop_front(class_inits);
    cg_inst_class_init(ctx, class_sym->class_ref, class_sym->sym);
    class_sym->sym = NULL;
    cg_class_sym_free(class_sym);
  }

  list_cg_class_sym_free(class_inits);
}
