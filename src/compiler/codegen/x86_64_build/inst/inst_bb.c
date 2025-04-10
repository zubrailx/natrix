#include "compiler/codegen/exception.h"
#include "compiler/codegen/x86_64/x86_64.h"
#include "compiler/mir/str.h"
#include "compiler/type_table/str.h"
#include "inst.h"
#include "util/log.h"
#include "util/macro.h"
#include "x86_64_core/value.h"
#include <stdarg.h>
#include <string.h>

static void cg_inst_frame_restore(cg_ctx *ctx, uint64_t frame_size_old) {
  if (frame_size_old != ctx->frame_size) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_ADDQ,
        cg_x86_64_op_new_immediate(ctx->frame_size - frame_size_old),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
    ctx->frame_size = frame_size_old;
  }
}

// if meta == NULL, push NULL
static void cg_inst_value_reg(cg_ctx *ctx, const cg_value_meta *meta,
                              cg_x86_64_reg reg) {
  if (meta) {
    if (meta->is_ptr) {
      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_MOVQ,
          cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
          cg_x86_64_op_new_register(reg), NULL);
    } else {
      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_LEAQ,
          cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
          cg_x86_64_op_new_register(reg), NULL);
    }
  } else {
    cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_MOVQ,
                                  cg_x86_64_op_new_immediate(0),
                                  cg_x86_64_op_new_register(reg), NULL);
  }
}

// if meta == NULL, pushes NULL
static void cg_inst_value_push(cg_ctx *ctx, const cg_value_meta *meta) {
  if (meta) {
    if (meta->is_ptr) {
      ctx->frame_size += cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_PUSHQ,
          cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP), NULL);
    } else {
      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_LEAQ,
          cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

      ctx->frame_size += cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_PUSHQ,
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);
    }
  } else {
    ctx->frame_size += cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_PUSHQ, cg_x86_64_op_new_immediate(0), NULL);
  }
}

// updates frame size, if value == NULL then need to add NULL as argument
static void cg_inst_call_pass_values(cg_ctx *ctx, uint64_t values_cnt,
                                     const mir_value *const *values) {

  // calculate memory that will be used for arguments and add alignment
  uint64_t frame_size_new =
      ctx->frame_size +
      ((values_cnt > 6) ? CG_X86_64_SIZE_QUAD * (values_cnt - 6) : 0);

  uint64_t alignment = cg_aligned(frame_size_new) - (frame_size_new);
  if (alignment) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_SUBQ, cg_x86_64_op_new_immediate(alignment),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
    frame_size_new += alignment;
  }

  // will overflow to uint64_max, prepares args from last to first
  for (uint64_t i = values_cnt - 1; i != UINT64_MAX; --i) {
    const mir_value     *value = values[i];
    const cg_value_meta *meta =
        value ? cg_ctx_value_meta_find(ctx, value) : NULL;

    switch (i) {
      case 0: {
        cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
        break;
      }
      case 1: {
        cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RSI);
        break;
      }
      case 2: {
        cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDX);
        break;
      }
      case 3: {
        cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RCX);
        break;
      }
      case 4: {
        cg_inst_value_reg(ctx, meta, CG_X86_64_REG_R8);
        break;
      }
      case 5: {
        cg_inst_value_reg(ctx, meta, CG_X86_64_REG_R9);
        break;
      }
      default: {
        cg_inst_value_push(ctx, meta);
        break;
      }
    }
  }

  ctx->frame_size = frame_size_new;
}

static void cg_inst_call_op_tbl(cg_ctx *ctx, const mir_value *self,
                                uint64_t offset) {
  const cg_value_meta *meta = cg_ctx_value_meta_find(ctx, self);

  if (meta->is_ptr) {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(meta->offset, CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(offsetof(x86_64_value, op_tbl),
                                  CG_X86_64_REG_RAX),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);
  } else {
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_MOVQ,
        cg_x86_64_op_new_base_imm(meta->offset + offsetof(x86_64_value, op_tbl),
                                  CG_X86_64_REG_RBP),
        cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);
  }

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_MOVQ,
      cg_x86_64_op_new_base_imm(offset, CG_X86_64_REG_RAX),
      cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                                NULL);
}

// if o_null is set then adds NULL as last elements
static uint64_t cg_inst_call_values(const mir_value    *ret,
                                    list_mir_value_ref *args,
                                    const mir_value ***values_out, int o_null) {
  uint64_t values_cnt = (ret ? 1 : 0) +
                        (args ? list_mir_value_ref_size(args) : 0) +
                        (o_null ? 1 : 0);

  const mir_value **values = MALLOCN(const mir_value *, values_cnt);
  {
    uint64_t i = 0;
    if (ret) {
      values[i++] = ret;
    }
    if (args) {
      for (list_mir_value_ref_it it = list_mir_value_ref_begin(args); !END(it);
           ++i, NEXT(it)) {
        values[i] = GET(it);
      }
    }
    if (o_null) {
      values[i] = NULL;
    }
  }

  *values_out = values;
  return values_cnt;
}

static void cg_inst_stmt_op(cg_ctx *ctx, const mir_stmt *stmt) {
  uint64_t frame_size_old = ctx->frame_size;

  const mir_value **values = NULL;
  uint64_t          values_cnt;

  const mir_value *self = list_mir_value_ref_front(stmt->op.args);
  if (!self) {
    error("expected any arg to be passed by op %s(%d) %p",
          mir_stmt_op_enum_str(stmt->op.kind), stmt->op.kind, stmt);
    goto cleanup;
  }

  switch (stmt->op.kind) {
    case MIR_STMT_OP_UNARY_PLUS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_plus));
      break;
    }
    case MIR_STMT_OP_UNARY_MINUS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_minus));
      break;
    }
    case MIR_STMT_OP_UNARY_LOGICAL_NOT: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_not));
      break;
    }
    case MIR_STMT_OP_UNARY_BITWISE_NOT: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_bit_not));
      break;
    }
    case MIR_STMT_OP_UNARY_INC: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_inc));
      break;
    }
    case MIR_STMT_OP_UNARY_DEC: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_dec));
      break;
    }
    case MIR_STMT_OP_BINARY_LOGICAL_OR: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_or));
      break;
    }
    case MIR_STMT_OP_BINARY_LOGICAL_AND: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_and));
      break;
    }
    case MIR_STMT_OP_BINARY_BITWISE_OR: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_bit_or));
      break;
    }
    case MIR_STMT_OP_BINARY_BITWISE_XOR: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_bit_xor));
      break;
    }
    case MIR_STMT_OP_BINARY_BITWISE_AND: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_bit_and));
      break;
    }
    case MIR_STMT_OP_BINARY_EQUALS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_eq));
      break;
    }
    case MIR_STMT_OP_BINARY_NOT_EQUALS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_neq));
      break;
    }
    case MIR_STMT_OP_BINARY_LESS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_less));
      break;
    }
    case MIR_STMT_OP_BINARY_LESS_EQUALS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_less_eq));
      break;
    }
    case MIR_STMT_OP_BINARY_GREATER: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      if (values_cnt >= 3) {
        const mir_value *tmp = values[1];
        values[1]            = values[2];
        values[2]            = tmp;
        cg_inst_call_pass_values(ctx, values_cnt, values);
        cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_less));
      } else {
        error("expected at least 3 arguments for %s(%d) to modify order %p",
              mir_stmt_op_enum_str(stmt->op.kind), stmt->op.kind, stmt);
      }
      break;
    }
    case MIR_STMT_OP_BINARY_GREATER_EQUALS: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      if (values_cnt >= 3) {
        const mir_value *tmp = values[1];
        values[1]            = values[2];
        values[2]            = tmp;
        cg_inst_call_pass_values(ctx, values_cnt, values);
        cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_less_eq));
      } else {
        error("expected at least 3 arguments for %s(%d) to modify order %p",
              mir_stmt_op_enum_str(stmt->op.kind), stmt->op.kind, stmt);
      }
      break;
    }
    case MIR_STMT_OP_BINARY_BITWISE_SHIFT_LEFT: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_bit_shl));
      break;
    }
    case MIR_STMT_OP_BINARY_BITWISE_SHIFT_RIGHT: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_bit_shr));
      break;
    }
    case MIR_STMT_OP_BINARY_ADD: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_add));
      break;
    }
    case MIR_STMT_OP_BINARY_SUB: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_sub));
      break;
    }
    case MIR_STMT_OP_BINARY_MUL: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_mul));
      break;
    }
    case MIR_STMT_OP_BINARY_DIV: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_div));
      break;
    }
    case MIR_STMT_OP_BINARY_REM: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_rem));
      break;
    }
    case MIR_STMT_OP_CALL: {
      cg_inst_call_pass_values(ctx, 1, &self);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_call));

      ctx->frame_size += cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_PUSHQ,
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

      uint64_t frame_size_rax = ctx->frame_size;

      // move values[0] to values[1] thus overwriting op_call
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      values[1]  = values[0];
      cg_inst_call_pass_values(ctx, values_cnt - 1, values + 1);

      uint64_t rax_offset = ctx->frame_size - frame_size_rax;

      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_MOVQ,
          cg_x86_64_op_new_base_imm(rax_offset, CG_X86_64_REG_RSP),
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_CALL,
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

      break;
    }
    case MIR_STMT_OP_INDEX: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 1);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_index));
      break;
    }
    case MIR_STMT_OP_INDEX_REF: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 1);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_index_ref));
      break;
    }
    case MIR_STMT_OP_DEREF: {
      values_cnt = cg_inst_call_values(stmt->op.ret, stmt->op.args, &values, 0);
      cg_inst_call_pass_values(ctx, values_cnt, values);
      cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_deref));
      break;
    }
    default:
      error("unhandled op kind %s(%d) %p", mir_stmt_op_enum_str(stmt->op.kind),
            stmt->op.kind, stmt);
      break;
  }

cleanup:
  cg_inst_frame_restore(ctx, frame_size_old);

  if (values) {
    free(values);
  }
}

static void cg_inst_stmt_call_extern(cg_ctx *ctx, const mir_stmt *stmt) {
  const mir_subroutine *sub     = stmt->call.sub;
  const char           *sub_sym = cg_ctx_mir_sym_find_sub(ctx, stmt->call.sub);
  int                   ok      = 1;

  if (!sub_sym) {
    return;
  }

  union {
    const type_base     *base;
    const type_callable *sub;
  } type;

  type.base = sub->type_ref->type;

  if (type.base->kind != TYPE_CALLABLE) {
    return;
  }

  // errors are generated on validation phase not to create duplications
  for (list_type_ref_it it = list_type_ref_begin(type.sub->params); !END(it);
       NEXT(it)) {
    const type_base *param_type = GET(it);
    if (!cg_inst_sub_extern_check_param(param_type)) {
      ok = 0;
    }
  }

  if (!cg_inst_sub_extern_check_ret(type.sub->ret_ref)) {
    ok = 0;
  }

  // get argc and check if it matches extern
  uint64_t args_cnt   = list_mir_value_ref_size(stmt->call.args);
  uint64_t params_cnt = list_type_ref_size(type.sub->params);

  if (args_cnt != params_cnt) {
    span *span = mir_debug_to_span(&stmt->debug);
    cg_exception_add_error(
        ctx->exceptions, EXCEPTION_CG_UNEXPECTED_ARGS, span,
        "args count doesn't match declaration: expected %lu, got %lu",
        params_cnt, args_cnt);
    span_free(span);
    ok = 0;
  }

  if (!ok) {
    return;
  }

  // allocate memory on stack required for argument unwrapping
  uint64_t frame_size_old = ctx->frame_size;

  ctx->frame_size =
      cg_aligned(ctx->frame_size + args_cnt * CG_X86_64_SIZE_QUAD);

  if (frame_size_old != ctx->frame_size) {
    uint64_t size = ctx->frame_size - frame_size_old;
    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_SUBQ, cg_x86_64_op_new_immediate(size),
        cg_x86_64_op_new_register(CG_X86_64_REG_RSP), NULL);
  }

  // store unwrapped values on stack
  {
    list_type_ref_it type_it = list_type_ref_begin(type.sub->params);
    uint64_t         arg_i   = 0;

    for (list_mir_value_ref_it it = list_mir_value_ref_begin(stmt->call.args);
         !END(it); NEXT(it), ++arg_i, NEXT(type_it)) {
      const mir_value     *arg  = GET(it);
      const cg_value_meta *meta = cg_ctx_value_meta_find(ctx, arg);
      const type_base     *type = GET(type_it);
      int                  ok   = 1;

      // should match validation
      switch (type->kind) {
        case TYPE_PRIMITIVE: {
          const type_primitive *prim = (typeof(prim))type;
          switch (prim->type) {
            case TYPE_PRIMITIVE_BOOL: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_bool")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_BYTE: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_byte")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_INT: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_int")), NULL);
              break;
            }
            case TYPE_PRIMITIVE_UINT: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_uint")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_LONG: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_long")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_ULONG: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_ulong")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_CHAR: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_char")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_STRING: {
              cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
              cg_ctx_text_emplace_back_text(
                  ctx, CG_X86_64_MNEM_CALL,
                  cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_string")),
                  NULL);
              break;
            }
            case TYPE_PRIMITIVE_VOID:
            case TYPE_PRIMITIVE_ANY:
              ok = 0;
              break;
          }
          break;
        }
        default:
          ok = 0;
          break;
      }

      if (!ok) {
        char *type_s = type_str(type);
        error("unexpected type %s", type_s);
        free(type_s);
        continue;
      }

      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_MOVQ,
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
          cg_x86_64_op_new_base_imm((uint64_t)CG_X86_64_SIZE_QUAD * arg_i,
                                    CG_X86_64_REG_RSP),
          NULL);
    }
  }

  // pop max 6 arguments from stack
  for (uint64_t i = 0; i < 6 && i < args_cnt; ++i) {
    switch (i) {
      case 0: {
        ctx->frame_size -= cg_ctx_text_emplace_back_text(
            ctx, CG_X86_64_MNEM_POPQ,
            cg_x86_64_op_new_register(CG_X86_64_REG_RDI), NULL);
        break;
      }
      case 1: {
        ctx->frame_size -= cg_ctx_text_emplace_back_text(
            ctx, CG_X86_64_MNEM_POPQ,
            cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
        break;
      }
      case 2: {
        ctx->frame_size -= cg_ctx_text_emplace_back_text(
            ctx, CG_X86_64_MNEM_POPQ,
            cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
        break;
      }
      case 3: {
        ctx->frame_size -= cg_ctx_text_emplace_back_text(
            ctx, CG_X86_64_MNEM_POPQ,
            cg_x86_64_op_new_register(CG_X86_64_REG_RCX), NULL);
        break;
      }
      case 4: {
        ctx->frame_size -= cg_ctx_text_emplace_back_text(
            ctx, CG_X86_64_MNEM_POPQ,
            cg_x86_64_op_new_register(CG_X86_64_REG_R8), NULL);
        break;
      }
      case 5: {
        ctx->frame_size -= cg_ctx_text_emplace_back_text(
            ctx, CG_X86_64_MNEM_POPQ,
            cg_x86_64_op_new_register(CG_X86_64_REG_R9), NULL);
        break;
      }
    }
  }

  // call
  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                cg_x86_64_op_new_direct(strdup(sub_sym)), NULL);

  // push result from RAX to resulting var
  {
    int              ok  = 1;
    const type_base *ret = type.sub->ret_ref;

    // should match validation
    switch (ret->kind) {
      case TYPE_PRIMITIVE: {
        const type_primitive *prim  = (typeof(prim))ret;
        const mir_value      *value = stmt->call.ret;
        const cg_value_meta  *meta  = cg_ctx_value_meta_find(ctx, value);

        switch (prim->type) {
          case TYPE_PRIMITIVE_BOOL: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVB,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_bool")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_BYTE: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVB,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_byte")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_INT: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVL,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_int")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_UINT: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVL,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_uint")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_LONG: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVQ,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_long")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_ULONG: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVQ,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_ulong")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_CHAR: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVB,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_char")), NULL);
            break;
          }
          case TYPE_PRIMITIVE_STRING: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_MOVQ,
                cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
                cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_string_move")),
                NULL);
            break;
          }
          case TYPE_PRIMITIVE_VOID: {
            cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);
            cg_ctx_text_emplace_back_text(
                ctx, CG_X86_64_MNEM_CALL,
                cg_x86_64_op_new_direct(strdup("__x86_64_make_void")), NULL);
          } break;
          case TYPE_PRIMITIVE_ANY:
            ok = 0;
            break;
        }
      } break;
      case TYPE_ARRAY:
      case TYPE_CALLABLE:
      case TYPE_CLASS_T:
      case TYPE_TYPENAME:
      case TYPE_MONO:
        ok = 0;
        break;
    }

    if (!ok) {
      char *type_s = type_str(ret);
      error("unexpected type %s", type_s);
      free(type_s);
    }
  }

  // restore stack frame
  cg_inst_frame_restore(ctx, frame_size_old);
}

static void cg_inst_stmt_call_native(cg_ctx *ctx, const mir_stmt *stmt) {
  const char *sub_sym = cg_ctx_mir_sym_find_sub(ctx, stmt->call.sub);
  if (!sub_sym) {
    error("subroutine not found in mir index");
    return;
  }

  const mir_value **values;
  uint64_t          values_cnt =
      cg_inst_call_values(stmt->call.ret, stmt->call.args, &values, 0);

  uint64_t frame_size_old = ctx->frame_size;

  cg_inst_call_pass_values(ctx, values_cnt, values);

  cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                cg_x86_64_op_new_direct(strdup(sub_sym)), NULL);

  cg_inst_frame_restore(ctx, frame_size_old);

  free(values);
}

static void cg_inst_stmt_call(cg_ctx *ctx, const mir_stmt *stmt) {
  int is_extern = stmt->call.sub->spec & MIR_SUBROUTINE_SPEC_EXTERN;

  if (is_extern) {
    cg_inst_stmt_call_extern(ctx, stmt);
  } else {
    cg_inst_stmt_call_native(ctx, stmt);
  }
}

static void cg_inst_stmt_member(cg_ctx *ctx, const mir_stmt *stmt, int o_ref) {
  const mir_lit   *member      = stmt->member.member;
  const type_base *member_type = member->type_ref->type;

  if (member_type->kind != TYPE_PRIMITIVE &&
      ((type_primitive *)member_type)->type != TYPE_PRIMITIVE_STRING) {
    char *type_s = type_str(member_type);
    error("expected member type TYPE_PRIMITIVE, STRING, got '%s'", type_s);
    free(type_s);
    return;
  }

  const char *member_sym = cg_ctx_mir_sym_find_lit(ctx, member);
  if (!member_sym) {
    char *sym = cg_ctx_mir_sym_emplace_lit(ctx, stmt->member.member);

    cg_ctx_data_push_back(ctx, cg_x86_64_symbol_new_data(sym));
    cg_ctx_data_push_back(ctx, cg_x86_64_data_new_ascii((uint8_t *)strdup(
                                   (char *)stmt->member.member->value.v_str)));

    member_sym = sym;
  }

  const mir_value     *ret      = stmt->member.ret;
  const cg_value_meta *ret_meta = cg_ctx_value_meta_find(ctx, ret);

  const mir_value     *self      = stmt->member.obj;
  const cg_value_meta *self_meta = cg_ctx_value_meta_find(ctx, self);

  cg_inst_value_reg(ctx, ret_meta, CG_X86_64_REG_RDI);
  cg_inst_value_reg(ctx, self_meta, CG_X86_64_REG_RSI);

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_LEAQ,
      cg_x86_64_op_new_base_sym(strdup(member_sym), CG_X86_64_REG_RIP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);

  if (o_ref) {
    cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_member_ref));
  } else {
    cg_inst_call_op_tbl(ctx, self, offsetof(x86_64_op_tbl, op_member));
  }
}

static void cg_inst_stmt_builtin_cast(cg_ctx *ctx, const mir_stmt *stmt) {
  if (list_mir_value_ref_size(stmt->builtin.args) != 1) {
    span *span = mir_debug_to_span(&stmt->debug);
    cg_exception_add_error(ctx->exceptions, EXCEPTION_CG_UNEXPECTED_ARGS, span,
                           "cast expected 1 argument");
    span_free(span);
    return;
  }

  const type_base *type = stmt->builtin.type ? stmt->builtin.type->type : NULL;

  if (!type) {
    span *span = mir_debug_to_span(&stmt->debug);
    cg_exception_add_error(ctx->exceptions, EXCEPTION_CG_UNEXPECTED_ARGS, span,
                           "no cast type specified");
    span_free(span);
    return;
  }

  const mir_value     *ret      = stmt->builtin.ret;
  const cg_value_meta *ret_meta = cg_ctx_value_meta_find(ctx, ret);

  const mir_value     *value = list_mir_value_ref_front(stmt->builtin.args);
  const cg_value_meta *value_meta = cg_ctx_value_meta_find(ctx, value);

  cg_inst_value_reg(ctx, ret_meta, CG_X86_64_REG_RDI);
  cg_inst_value_reg(ctx, value_meta, CG_X86_64_REG_RSI);

  int ok = 1;
  switch (type->kind) {
    case TYPE_PRIMITIVE: {
      const type_primitive *prim = (typeof(prim))type;

      switch (prim->type) {
        case TYPE_PRIMITIVE_BOOL: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_BOOL),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_BYTE: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_BYTE),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_INT: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_INT),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_UINT: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_UINT),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_LONG: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_LONG),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_ULONG: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_ULONG),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_CHAR: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_CHAR),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_STRING: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_STRING),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        case TYPE_PRIMITIVE_VOID: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(X86_64_TYPE_VOID),
              cg_x86_64_op_new_register(CG_X86_64_REG_RDX), NULL);
          break;
        }
        default: {
          ok = 0;
          break;
        }
      }
      break;
    }
    case TYPE_ARRAY:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME:
    case TYPE_MONO:
    default: {
      ok = 0;
      break;
    }
  }

  if (!ok) {
    char *type_s = type_str(type);
    span *span   = mir_debug_to_span(&stmt->debug);
    cg_exception_add_error(ctx->exceptions, EXCEPTION_CG_UNEXPECTED_ARGS, span,
                           "can't cast to type %s, unimplemented", type_s);
    span_free(span);
    if (type_s) {
      free(type_s);
    }
    return;
  }

  cg_inst_call_op_tbl(ctx, value, offsetof(x86_64_op_tbl, op_cast));
}

static void cg_inst_stmt_builtin_make(cg_ctx *ctx, const mir_stmt *stmt) {
  const type_base *type = stmt->builtin.type->type;

  switch (type->kind) {
    case TYPE_PRIMITIVE: {
      char *type_s = type_str(type);
      span *span   = mir_debug_to_span(&stmt->debug);
      cg_exception_add_error(
          ctx->exceptions, EXCEPTION_CG_UNEXPECTED_TYPE, span,
          "primitive type '%s' cannot be instantiated with make", type_s);
      span_free(span);
      free(type_s);
      break;
    }
    case TYPE_ARRAY: {
      const type_array *array = (typeof(array))type;

      // calculate array depth
      uint64_t depth = 0;
      {
        union {
          const type_base  *base;
          const type_array *arr;
        } tu;

        tu.base = type;

        while (tu.base->kind == TYPE_ARRAY) {
          tu.base = tu.arr->element_ref;
          ++depth;
        }
      }

      uint64_t args_cnt = list_mir_value_ref_size(stmt->builtin.args);
      if (args_cnt != depth) {
        span *span = mir_debug_to_span(&stmt->debug);
        cg_exception_add_error(
            ctx->exceptions, EXCEPTION_CG_UNEXPECTED_ARGS, span,
            "array args count mismatch array depth: expected %lu, got %lu",
            depth, args_cnt);
        span_free(span);
        break;
      }

      uint64_t frame_size_old = ctx->frame_size;

      const mir_value **values;
      const uint64_t    values_cnt = cg_inst_call_values(
          stmt->builtin.ret, stmt->builtin.args, &values, 1);

      cg_inst_call_pass_values(ctx, values_cnt, values);

      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_CALL,
          cg_x86_64_op_new_direct(strdup("__x86_64_make_array")), NULL);

      cg_inst_frame_restore(ctx, frame_size_old);

      free(values);
      break;
    }
    case TYPE_MONO: {
      const type_mono *mono = (typeof(mono))type;
      switch (mono->type_ref->kind) {
        case TYPE_CLASS_T:
          if (!list_mir_value_ref_empty(stmt->builtin.args)) {
            span *span   = mir_debug_to_span(&stmt->debug);
            char *type_s = type_str(type);
            cg_exception_add_warning(ctx->exceptions,
                                     EXCEPTION_CG_UNEXPECTED_ARGS, span,
                                     "make type '%s' args are ignored", type_s);
            span_free(span);
            free(type_s);
          }

          const mir_value     *out  = stmt->builtin.ret;
          const cg_value_meta *meta = cg_ctx_value_meta_find(ctx, out);
          cg_inst_value_reg(ctx, meta, CG_X86_64_REG_RDI);

          const char *sym = cg_ctx_type_sym_init_find_class(ctx, mono);

          if (!sym) {
            char *type_s = type_str(type);
            error("type '%s' initializer not found", type_s);
            free(type_s);
            break;
          }

          cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_CALL,
                                        cg_x86_64_op_new_direct(strdup(sym)),
                                        NULL);
          break;
        case TYPE_PRIMITIVE:
        case TYPE_ARRAY:
        case TYPE_CALLABLE:
        case TYPE_TYPENAME:
        case TYPE_MONO:
        default: {
          span *span   = mir_debug_to_span(&stmt->debug);
          char *type_s = type_str(type);
          cg_exception_add_error(ctx->exceptions, EXCEPTION_CG_UNEXPECTED_TYPE,
                                 span, "cannot make type '%s'", type_s);
          span_free(span);
          free(type_s);
          break;
        }
      }

      break;
    }
    default:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME: {
      char *type_s = type_str(type);
      error("unexpected type '%s' with kind %d for make expression", type_s,
            type->kind);
      free(type_s);
      break;
    }
  }
}

static void cg_inst_stmt_builtin_print(cg_ctx *ctx, const mir_stmt *stmt) {
  for (list_mir_value_ref_it it = list_mir_value_ref_begin(stmt->builtin.args);
       !END(it); NEXT(it)) {
    const mir_value     *value      = GET(it);
    const cg_value_meta *value_meta = cg_ctx_value_meta_find(ctx, value);

    cg_inst_value_reg(ctx, value_meta, CG_X86_64_REG_RDI);

    cg_ctx_text_emplace_back_text(
        ctx, CG_X86_64_MNEM_CALL,
        cg_x86_64_op_new_direct(strdup("__x86_64_print")), NULL);
  }
}

static void cg_inst_stmt_builtin_type(cg_ctx *ctx, const mir_stmt *stmt) {
  if (list_mir_value_ref_size(stmt->builtin.args) != 1) {
    span *span = mir_debug_to_span(&stmt->debug);
    cg_exception_add_error(ctx->exceptions, EXCEPTION_CG_UNEXPECTED_ARGS, span,
                           "cast expected 1 argument");
    span_free(span);
    return;
  }

  const mir_value     *ret      = stmt->builtin.ret;
  const cg_value_meta *ret_meta = cg_ctx_value_meta_find(ctx, ret);

  const mir_value     *value = list_mir_value_ref_front(stmt->builtin.args);
  const cg_value_meta *value_meta = cg_ctx_value_meta_find(ctx, value);

  cg_inst_value_reg(ctx, ret_meta, CG_X86_64_REG_RDI);
  cg_inst_value_reg(ctx, value_meta, CG_X86_64_REG_RSI);

  cg_inst_call_op_tbl(ctx, value, offsetof(x86_64_op_tbl, op_type));
}

static void cg_inst_stmt_builtin(cg_ctx *ctx, const mir_stmt *stmt) {
  switch (stmt->builtin.kind) {
    case MIR_STMT_BUILTIN_CAST: {
      return cg_inst_stmt_builtin_cast(ctx, stmt);
    }
    case MIR_STMT_BUILTIN_MAKE: {
      return cg_inst_stmt_builtin_make(ctx, stmt);
    }
    case MIR_STMT_BUILTIN_PRINT: {
      return cg_inst_stmt_builtin_print(ctx, stmt);
    }
    case MIR_STMT_BUILTIN_TYPE: {
      return cg_inst_stmt_builtin_type(ctx, stmt);
    }
  }
  error("unhandled builtin kind %d %p", stmt->builtin.kind, stmt);
}

static void cg_inst_stmt_assign_lit(cg_ctx *ctx, const mir_stmt *stmt) {
  const mir_value *to_value = stmt->assign.to;
  const mir_lit   *from_lit = stmt->assign.from_lit;

  const type_base *type = from_lit->type_ref->type;

  const cg_value_meta *to_meta = cg_ctx_value_meta_find(ctx, to_value);

  cg_inst_value_reg(ctx, to_meta, CG_X86_64_REG_RDI);

  switch (type->kind) {
    case TYPE_PRIMITIVE: {
      const type_primitive *prim = (typeof(prim))type;
      switch (prim->type) {
        case TYPE_PRIMITIVE_BOOL: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVB,
              cg_x86_64_op_new_immediate(from_lit->value.v_bool),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_bool")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_BYTE: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVB,
              cg_x86_64_op_new_immediate(from_lit->value.v_byte),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_byte")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_INT: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(from_lit->value.v_int),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_int")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_UINT: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVL,
              cg_x86_64_op_new_immediate(from_lit->value.v_uint),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_uint")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_LONG: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVQ,
              cg_x86_64_op_new_immediate(from_lit->value.v_long),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_long")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_ULONG: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVQ,
              cg_x86_64_op_new_immediate(from_lit->value.v_ulong),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_ulong")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_CHAR: {
          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_MOVB,
              cg_x86_64_op_new_immediate(from_lit->value.v_byte),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_char")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_STRING: {
          const char *sym_ref = cg_ctx_mir_sym_find_lit(ctx, from_lit);

          // insert new lit into index and add it to data code
          if (!sym_ref) {
            char *sym = cg_ctx_mir_sym_emplace_lit(ctx, from_lit);

            cg_ctx_data_push_back(ctx, cg_x86_64_symbol_new_data(sym));
            cg_ctx_data_push_back(
                ctx, cg_x86_64_data_new_ascii(
                         (uint8_t *)strdup((char *)from_lit->value.v_str)));

            sym_ref = sym;
          }

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_LEAQ,
              cg_x86_64_op_new_base_sym(strdup(sym_ref), CG_X86_64_REG_RIP),
              cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

          cg_ctx_text_emplace_back_text(
              ctx, CG_X86_64_MNEM_CALL,
              cg_x86_64_op_new_direct(strdup("__x86_64_make_string")), NULL);
          break;
        }
        case TYPE_PRIMITIVE_VOID:
        default: {
          error("unsupported lit prim type %d %p", prim->type, stmt);
          return;
        }
      }
      break;
    }
    case TYPE_ARRAY:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME:
    case TYPE_MONO:
    default: {
      error("unsupported lit type %s(%d) %p", type_enum_str(type->kind),
            type->kind, stmt);
      return;
    }
  }
}

static void cg_inst_stmt_assign_value(cg_ctx *ctx, const mir_stmt *stmt) {
  const mir_value *to_value   = stmt->assign.to;
  const mir_value *from_value = stmt->assign.from_value;

  const mir_value *values[] = {to_value, from_value};

  uint64_t frame_size_old = ctx->frame_size;

  cg_inst_call_pass_values(ctx, sizeof(values) / sizeof(values[0]), values);

  cg_inst_call_op_tbl(ctx, to_value, offsetof(x86_64_op_tbl, op_assign));

  cg_inst_frame_restore(ctx, frame_size_old);
}

static void cg_inst_stmt_assign_sub(cg_ctx *ctx, const mir_stmt *stmt) {
  const mir_value      *to_value = stmt->assign.to;
  const mir_subroutine *from_sub = stmt->assign.from_sub;

  const cg_value_meta *to_meta = cg_ctx_value_meta_find(ctx, to_value);
  const char          *sym_ref = cg_ctx_mir_sym_find_sub(ctx, from_sub);

  if (!sym_ref) {
    error("can't find mir subroutine %p in ctx", from_sub);
    return;
  }

  if (from_sub->spec & MIR_SUBROUTINE_SPEC_EXTERN) {
    span *span = mir_debug_to_span(&stmt->debug);
    cg_exception_add_error(
        ctx->exceptions, EXCEPTION_CG_EXTERN_SUBROUTINE_ASSIGNMENT, span,
        "can't assign external subroutine '%s' to variable, unimplemented",
        sym_ref);
    span_free(span);
    return;
  }

  cg_inst_value_reg(ctx, to_meta, CG_X86_64_REG_RDI);

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_LEAQ,
      cg_x86_64_op_new_base_sym(strdup(sym_ref), CG_X86_64_REG_RIP),
      cg_x86_64_op_new_register(CG_X86_64_REG_RSI), NULL);

  cg_ctx_text_emplace_back_text(
      ctx, CG_X86_64_MNEM_CALL,
      cg_x86_64_op_new_direct(strdup("__x86_64_make_callable")), NULL);
}

static void cg_inst_stmt_assign(cg_ctx *ctx, const mir_stmt *stmt) {
  switch (stmt->assign.kind) {
    case MIR_STMT_ASSIGN_LIT: {
      return cg_inst_stmt_assign_lit(ctx, stmt);
    }
    case MIR_STMT_ASSIGN_VALUE: {
      return cg_inst_stmt_assign_value(ctx, stmt);
    }
    case MIR_STMT_ASSIGN_SUB: {
      return cg_inst_stmt_assign_sub(ctx, stmt);
    }
  }
  error("unhandled assign kind %d", stmt->assign.kind);
}

static void cg_inst_stmt(cg_ctx *ctx, const mir_stmt *stmt) {

  if (cg_debug_enabled(ctx->debug)) {
    char *sym = cg_sym_local_suf_idx(ctx->sub_sym, "L", ctx->line_cnt++);

    cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sym));

    list_cg_debug_line_push_back(
        ctx->debug->lines,
        cg_debug_line_new(sym, stmt->debug.source_ref, stmt->debug.line));
  }

  switch (stmt->kind) {
    case MIR_STMT_OP:
      return cg_inst_stmt_op(ctx, stmt);
    case MIR_STMT_CALL:
      return cg_inst_stmt_call(ctx, stmt);
    case MIR_STMT_MEMBER:
      return cg_inst_stmt_member(ctx, stmt, 0);
    case MIR_STMT_MEMBER_REF:
      return cg_inst_stmt_member(ctx, stmt, 1);
    case MIR_STMT_BUILTIN:
      return cg_inst_stmt_builtin(ctx, stmt);
    case MIR_STMT_ASSIGN:
      return cg_inst_stmt_assign(ctx, stmt);
  }
  error("unhandled stmt kind %d %p", stmt->kind, stmt);
}

static void cg_inst_bb(cg_ctx *ctx, const mir_bb *bb) {
  ctx->bb = bb;

  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_text(cg_sym_local_bb(ctx->sub_sym, bb)));

  for (list_mir_stmt_it it = list_mir_stmt_begin(bb->stmts); !END(it);
       NEXT(it)) {
    const mir_stmt *stmt = GET(it);
    cg_inst_stmt(ctx, stmt);
  }

  if (cg_debug_enabled(ctx->debug) && bb->jmp.debug.source_ref) {
    char *sym = cg_sym_local_suf_idx(ctx->sub_sym, "L", ctx->line_cnt++);
    cg_ctx_text_push_back(ctx, cg_x86_64_symbol_new_text(sym));
    list_cg_debug_line_push_back(
        ctx->debug->lines,
        cg_debug_line_new(sym, bb->jmp.debug.source_ref, bb->jmp.debug.line));
  }

  mir_bb_enum kind = mir_bb_get_cond(bb);
  switch (kind) {
    // unwrap as bool and jump if true, else not jump
    case MIR_BB_COND: {
      const cg_value_meta *value_meta =
          cg_ctx_value_meta_find(ctx, bb->jmp.cond_ref);

      cg_inst_value_reg(ctx, value_meta, CG_X86_64_REG_RDI);

      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_CALL,
          cg_x86_64_op_new_direct(strdup("__x86_64_unwrap_bool")), NULL);

      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_TESTB,
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX),
          cg_x86_64_op_new_register(CG_X86_64_REG_RAX), NULL);

      cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_JNZ,
                                    cg_x86_64_op_new_direct(cg_sym_local_bb(
                                        ctx->sub_sym, bb->jmp.je_ref)),
                                    NULL);

      cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_JMP,
                                    cg_x86_64_op_new_direct(cg_sym_local_bb(
                                        ctx->sub_sym, bb->jmp.jz_ref)),
                                    NULL);
      break;
    }
    case MIR_BB_NEXT: {
      cg_ctx_text_emplace_back_text(ctx, CG_X86_64_MNEM_JMP,
                                    cg_x86_64_op_new_direct(cg_sym_local_bb(
                                        ctx->sub_sym, bb->jmp.next_ref)),
                                    NULL);
      break;
    }
    case MIR_BB_TERM: {
      cg_ctx_text_emplace_back_text(
          ctx, CG_X86_64_MNEM_JMP,
          cg_x86_64_op_new_direct(cg_sym_local_suf(ctx->sub_sym, "deinit")),
          NULL);
      break;
    }
    case MIR_BB_UNKNOWN:
    default:
      error("unknown bb kind %d %p", kind, bb);
      break;
  }
}

void cg_inst_bbs(cg_ctx *ctx, const list_mir_bb *bbs) {
  for (list_mir_bb_it it = list_mir_bb_begin(bbs); !END(it); NEXT(it)) {
    const mir_bb *bb = GET(it);
    cg_inst_bb(ctx, bb);
  }
}
