#include "emit.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include <string.h>

typedef struct cg_ctx_struct {
  strbuf         *buffer;
  list_exception *exceptions;
} cg_ctx;

static void cg_ctx_init(cg_ctx *ctx, strbuf *buffer,
                        list_exception *exceptions) {
  ctx->buffer     = buffer;
  ctx->exceptions = exceptions;
}

static void cg_ctx_deinit(cg_ctx *ctx) {
  ctx->buffer     = NULL;
  ctx->exceptions = NULL;
}

static void cg_emit_reg(cg_ctx *ctx, cg_x86_64_reg reg, uint64_t size) {
  switch (size) {
    case 1:
      switch (reg) {
        case CG_X86_64_REG_RAX:
          strbuf_append(ctx->buffer, "%al");
          break;
        case CG_X86_64_REG_RBX:
          strbuf_append(ctx->buffer, "%bl");
          break;
        case CG_X86_64_REG_RCX:
          strbuf_append(ctx->buffer, "%cl");
          break;
        case CG_X86_64_REG_RDX:
          strbuf_append(ctx->buffer, "%dl");
          break;
        case CG_X86_64_REG_RSP:
          strbuf_append(ctx->buffer, "%spl");
          break;
        case CG_X86_64_REG_RBP:
          strbuf_append(ctx->buffer, "%bpl");
          break;
        case CG_X86_64_REG_RSI:
          strbuf_append(ctx->buffer, "%sil");
          break;
        case CG_X86_64_REG_RDI:
          strbuf_append(ctx->buffer, "%dil");
          break;
        case CG_X86_64_REG_R8:
          strbuf_append(ctx->buffer, "%r8b");
          break;
        case CG_X86_64_REG_R9:
          strbuf_append(ctx->buffer, "%r9b");
          break;
        case CG_X86_64_REG_R10:
          strbuf_append(ctx->buffer, "%r10b");
          break;
        case CG_X86_64_REG_R11:
          strbuf_append(ctx->buffer, "%r11b");
          break;
        case CG_X86_64_REG_R12:
          strbuf_append(ctx->buffer, "%r12b");
          break;
        case CG_X86_64_REG_R13:
          strbuf_append(ctx->buffer, "%r13b");
          break;
        case CG_X86_64_REG_R14:
          strbuf_append(ctx->buffer, "%r14b");
          break;
        case CG_X86_64_REG_R15:
          strbuf_append(ctx->buffer, "%r15b");
          break;
        default:
          error("unhandled reg size %lu with reg %d", size, reg);
          break;
      }
      break;
    case 2:
      switch (reg) {
        case CG_X86_64_REG_RAX:
          strbuf_append(ctx->buffer, "%ax");
          break;
        case CG_X86_64_REG_RBX:
          strbuf_append(ctx->buffer, "%bx");
          break;
        case CG_X86_64_REG_RCX:
          strbuf_append(ctx->buffer, "%cx");
          break;
        case CG_X86_64_REG_RDX:
          strbuf_append(ctx->buffer, "%dx");
          break;
        case CG_X86_64_REG_RSP:
          strbuf_append(ctx->buffer, "%sp");
          break;
        case CG_X86_64_REG_RBP:
          strbuf_append(ctx->buffer, "%bp");
          break;
        case CG_X86_64_REG_RSI:
          strbuf_append(ctx->buffer, "%si");
          break;
        case CG_X86_64_REG_RDI:
          strbuf_append(ctx->buffer, "%di");
          break;
        case CG_X86_64_REG_R8:
          strbuf_append(ctx->buffer, "%r8w");
          break;
        case CG_X86_64_REG_R9:
          strbuf_append(ctx->buffer, "%r9w");
          break;
        case CG_X86_64_REG_R10:
          strbuf_append(ctx->buffer, "%r10w");
          break;
        case CG_X86_64_REG_R11:
          strbuf_append(ctx->buffer, "%r11w");
          break;
        case CG_X86_64_REG_R12:
          strbuf_append(ctx->buffer, "%r12w");
          break;
        case CG_X86_64_REG_R13:
          strbuf_append(ctx->buffer, "%r13w");
          break;
        case CG_X86_64_REG_R14:
          strbuf_append(ctx->buffer, "%r14w");
          break;
        case CG_X86_64_REG_R15:
          strbuf_append(ctx->buffer, "%r15w");
          break;
        default:
          error("unhandled reg size %lu with reg %d", size, reg);
          break;
      }
      break;
    case 4:
      switch (reg) {
        case CG_X86_64_REG_RAX:
          strbuf_append(ctx->buffer, "%eax");
          break;
        case CG_X86_64_REG_RBX:
          strbuf_append(ctx->buffer, "%ebx");
          break;
        case CG_X86_64_REG_RCX:
          strbuf_append(ctx->buffer, "%ecx");
          break;
        case CG_X86_64_REG_RDX:
          strbuf_append(ctx->buffer, "%edx");
          break;
        case CG_X86_64_REG_RSP:
          strbuf_append(ctx->buffer, "%esp");
          break;
        case CG_X86_64_REG_RBP:
          strbuf_append(ctx->buffer, "%ebp");
          break;
        case CG_X86_64_REG_RSI:
          strbuf_append(ctx->buffer, "%esi");
          break;
        case CG_X86_64_REG_RDI:
          strbuf_append(ctx->buffer, "%edi");
          break;
        case CG_X86_64_REG_R8:
          strbuf_append(ctx->buffer, "%r8d");
          break;
        case CG_X86_64_REG_R9:
          strbuf_append(ctx->buffer, "%r9d");
          break;
        case CG_X86_64_REG_R10:
          strbuf_append(ctx->buffer, "%r10d");
          break;
        case CG_X86_64_REG_R11:
          strbuf_append(ctx->buffer, "%r11d");
          break;
        case CG_X86_64_REG_R12:
          strbuf_append(ctx->buffer, "%r12d");
          break;
        case CG_X86_64_REG_R13:
          strbuf_append(ctx->buffer, "%r13d");
          break;
        case CG_X86_64_REG_R14:
          strbuf_append(ctx->buffer, "%r14d");
          break;
        case CG_X86_64_REG_R15:
          strbuf_append(ctx->buffer, "%r15d");
          break;
        default:
          error("unhandled reg size %lu with reg %d", size, reg);
          break;
      }
      break;
    case 8:
      switch (reg) {
        case CG_X86_64_REG_RAX:
          strbuf_append(ctx->buffer, "%rax");
          break;
        case CG_X86_64_REG_RBX:
          strbuf_append(ctx->buffer, "%rbx");
          break;
        case CG_X86_64_REG_RCX:
          strbuf_append(ctx->buffer, "%rcx");
          break;
        case CG_X86_64_REG_RDX:
          strbuf_append(ctx->buffer, "%rdx");
          break;
        case CG_X86_64_REG_RSP:
          strbuf_append(ctx->buffer, "%rsp");
          break;
        case CG_X86_64_REG_RBP:
          strbuf_append(ctx->buffer, "%rbp");
          break;
        case CG_X86_64_REG_RSI:
          strbuf_append(ctx->buffer, "%rsi");
          break;
        case CG_X86_64_REG_RDI:
          strbuf_append(ctx->buffer, "%rdi");
          break;
        case CG_X86_64_REG_R8:
          strbuf_append(ctx->buffer, "%r8");
          break;
        case CG_X86_64_REG_R9:
          strbuf_append(ctx->buffer, "%r9");
          break;
        case CG_X86_64_REG_R10:
          strbuf_append(ctx->buffer, "%r10");
          break;
        case CG_X86_64_REG_R11:
          strbuf_append(ctx->buffer, "%r11");
          break;
        case CG_X86_64_REG_R12:
          strbuf_append(ctx->buffer, "%r12");
          break;
        case CG_X86_64_REG_R13:
          strbuf_append(ctx->buffer, "%r13");
          break;
        case CG_X86_64_REG_R14:
          strbuf_append(ctx->buffer, "%r14");
          break;
        case CG_X86_64_REG_R15:
          strbuf_append(ctx->buffer, "%r15");
          break;
        case CG_X86_64_REG_RIP:
          strbuf_append(ctx->buffer, "%rip");
          break;
        default:
          error("unhandled reg size %lu with reg %d", size, reg);
          break;
      }
      break;
    default:
      error("unhandled reg size %lu with reg %d", size, reg);
      break;
  }
}

static void cg_emit_unit_data(cg_ctx *ctx, const cg_x86_64_data *data) {
  char buf[64];

  switch (data->kind) {
    case CG_X86_64_DATA_BYTE:
      strbuf_append_f(ctx->buffer, buf, ".byte %#x", data->data_byte);
      break;
    case CG_X86_64_DATA_WORD:
      strbuf_append_f(ctx->buffer, buf, ".word %#x", data->data_word);
      break;
    case CG_X86_64_DATA_LONG:
      strbuf_append_f(ctx->buffer, buf, ".long %#x", data->data_long);
      break;
    case CG_X86_64_DATA_QUAD:
      strbuf_append_f(ctx->buffer, buf, ".quad %#lx", data->data_quad);
      break;
    case CG_X86_64_DATA_ASCII:
      strbuf_append(ctx->buffer, ".ascii ");
      strbuf_append(ctx->buffer, "\"");
      strbuf_append(ctx->buffer, (char *)data->data_ascii);
      strbuf_append(ctx->buffer, "\\0\"");
      break;
    case CG_X86_64_DATA_BYTES:
      strbuf_append(ctx->buffer, ".byte ");
      size_t i = 0;
      if (i++ < data->data_len) {
        strbuf_append_f(ctx->buffer, buf, "%#x", data->data_bytes[i]);
      }
      for (; i < data->data_len; ++i) {
        strbuf_append_f(ctx->buffer, buf, ", %#x", data->data_bytes[i]);
      }
      break;
    case CG_X86_64_DATA_SYMBOL:
      strbuf_append(ctx->buffer, ".quad ");
      strbuf_append(ctx->buffer, data->data_symbol);
      break;
    default:
      error("unexpected data kind %d %p", data->kind, data);
  }

  strbuf_append(ctx->buffer, "\n");
}

static void cg_emit_unit_text_op(cg_ctx *ctx, cg_x86_64_mnem mnem,
                                 const cg_x86_64_op *op, uint64_t size) {
  char buf[64];

  // add * for ca
  switch (mnem) {
    case CG_X86_64_MNEM_CALL:
    case CG_X86_64_MNEM_JZ:
    case CG_X86_64_MNEM_JNZ:
    case CG_X86_64_MNEM_JMP:
      switch (op->kind) {
        case CG_X86_64_MODE_REGISTER:
        case CG_X86_64_MODE_INDEXED:
        case CG_X86_64_MODE_INDIRECT:
        case CG_X86_64_MODE_BASE_IMM:
        case CG_X86_64_MODE_BASE_SYM:
          strbuf_append(ctx->buffer, "*");
          break;
        default:
          break;
      }
    default:
      break;
  }

  switch (op->kind) {
    case CG_X86_64_MODE_REGISTER:
      cg_emit_reg(ctx, op->reg.reg, size);
      break;
    case CG_X86_64_MODE_DIRECT:
      strbuf_append(ctx->buffer, op->direct.sym_addr);
      break;
    case CG_X86_64_MODE_INDEXED:
      strbuf_append(ctx->buffer, op->indexed.sym_addr);
      strbuf_append(ctx->buffer, "(,");
      cg_emit_reg(ctx, op->indexed.reg_index, size);
      strbuf_append_f(ctx->buffer, buf, ",%#lx)", op->indexed.imm_multi);
      break;
    case CG_X86_64_MODE_INDIRECT:
      strbuf_append(ctx->buffer, "(");
      cg_emit_reg(ctx, op->indirect.reg_base, size);
      strbuf_append(ctx->buffer, ")");
      break;
    case CG_X86_64_MODE_BASE_IMM:
      if ((int64_t)op->base_imm.imm_offset < 0) {
        strbuf_append_f(ctx->buffer, buf, "-%#lx", -op->base_imm.imm_offset);
      } else {
        strbuf_append_f(ctx->buffer, buf, "%#lx", op->base_imm.imm_offset);
      }
      strbuf_append(ctx->buffer, "(");
      cg_emit_reg(ctx, op->base_imm.reg_base, size);
      strbuf_append(ctx->buffer, ")");
      break;
    case CG_X86_64_MODE_BASE_SYM:
      strbuf_append(ctx->buffer, op->base_sym.sym_addr);
      strbuf_append(ctx->buffer, "(");
      cg_emit_reg(ctx, op->base_sym.reg_base, size);
      strbuf_append(ctx->buffer, ")");
      break;
    case CG_X86_64_MODE_IMMEDIATE:
      strbuf_append_f(ctx->buffer, buf, "$%#lx", op->imm.imm_const);
      break;
    default:
      error("unhandled mode op %d %p", op->kind, op);
      break;
  }
}

static void cg_emit_unit_text(cg_ctx *ctx, const cg_x86_64_text *text) {
  // padding
  strbuf_append(ctx->buffer, "  ");

  switch (text->mnem) {
    case CG_X86_64_MNEM_PUSHQ:
      strbuf_append(ctx->buffer, "pushq");
      break;
    case CG_X86_64_MNEM_POPQ:
      strbuf_append(ctx->buffer, "popq");
      break;
    case CG_X86_64_MNEM_MOVB:
      strbuf_append(ctx->buffer, "movb");
      break;
    case CG_X86_64_MNEM_MOVL:
      strbuf_append(ctx->buffer, "movl");
      break;
    case CG_X86_64_MNEM_MOVQ:
      strbuf_append(ctx->buffer, "movq");
      break;
    case CG_X86_64_MNEM_RETQ:
      strbuf_append(ctx->buffer, "retq");
      break;
    case CG_X86_64_MNEM_SYSCALL:
      strbuf_append(ctx->buffer, "syscall");
      break;
    case CG_X86_64_MNEM_XORQ:
      strbuf_append(ctx->buffer, "xorq");
      break;
    case CG_X86_64_MNEM_SUBQ:
      strbuf_append(ctx->buffer, "subq");
      break;
    case CG_X86_64_MNEM_ADDQ:
      strbuf_append(ctx->buffer, "addq");
      break;
    case CG_X86_64_MNEM_LEAQ:
      strbuf_append(ctx->buffer, "leaq");
      break;
    case CG_X86_64_MNEM_CALL:
      strbuf_append(ctx->buffer, "call");
      break;
    case CG_X86_64_MNEM_TESTB:
      strbuf_append(ctx->buffer, "testb");
      break;
    case CG_X86_64_MNEM_JZ:
      strbuf_append(ctx->buffer, "jz");
      break;
    case CG_X86_64_MNEM_JNZ:
      strbuf_append(ctx->buffer, "jnz");
      break;
    case CG_X86_64_MNEM_JMP:
      strbuf_append(ctx->buffer, "jmp");
      break;
    default:
      error("unhandled mnem %d %p", text->mnem, text);
      break;
  }

  uint64_t op_size = cg_x86_64_mnem_size(text->mnem);

  list_cg_x86_64_op_it it = list_cg_x86_64_op_begin(text->operands);
  if (!END(it)) {
    strbuf_append(ctx->buffer, " ");
    cg_emit_unit_text_op(ctx, text->mnem, GET(it), op_size);
  }
  for (NEXT(it); !END(it); NEXT(it)) {
    strbuf_append(ctx->buffer, ", ");
    cg_emit_unit_text_op(ctx, text->mnem, GET(it), op_size);
  }

  strbuf_append(ctx->buffer, "\n");
}

static void cg_emit_unit_symbol(cg_ctx *ctx, const cg_x86_64_symbol *symbol) {
  switch (symbol->kind) {
    case CG_X86_64_SYMBOL_DATA:
    case CG_X86_64_SYMBOL_DATA_LN:
      strbuf_append(ctx->buffer, symbol->name);
      strbuf_append(ctx->buffer, ": ");
      if (symbol->kind == CG_X86_64_SYMBOL_DATA_LN) {
        strbuf_append(ctx->buffer, "\n");
      }
      break;
    case CG_X86_64_SYMBOL_TEXT:
      strbuf_append(ctx->buffer, symbol->name);
      strbuf_append(ctx->buffer, ":\n");
      break;
    case CG_X86_64_SYMBOL_EXTERN:
      strbuf_append(ctx->buffer, ".extern ");
      strbuf_append(ctx->buffer, symbol->name);
      strbuf_append(ctx->buffer, "\n");
      break;
    case CG_X86_64_SYMBOL_GLOBAL:
      strbuf_append(ctx->buffer, ".globl ");
      strbuf_append(ctx->buffer, symbol->name);
      strbuf_append(ctx->buffer, "\n");
      break;
    default:
      error("unhandled symbol kind %d %p", symbol->kind, symbol);
      break;
  }
}

static void cg_emit_section(cg_ctx *ctx, const char *section_name,
                            const list_cg_x86_64_unit *units) {
  strbuf_append(ctx->buffer, ".section ");
  strbuf_append(ctx->buffer, section_name);
  strbuf_append(ctx->buffer, "\n");

  for (list_cg_x86_64_unit_it it = list_cg_x86_64_unit_begin(units); !END(it);
       NEXT(it)) {
    const cg_x86_64_unit *unit = GET(it);

    switch (unit->kind) {
      case CG_X86_64_UNIT_DATA:
        cg_emit_unit_data(ctx, (cg_x86_64_data *)unit);
        break;
      case CG_X86_64_UNIT_TEXT:
        cg_emit_unit_text(ctx, (cg_x86_64_text *)unit);
        break;
      case CG_X86_64_UNIT_SYMBOL:
        cg_emit_unit_symbol(ctx, (cg_x86_64_symbol *)unit);
        break;
      default:
        error("unexpected unit kind %d %p", unit->kind, unit);
        break;
    }
  }
}

char *cg_x86_64_emit_gas(cg_x86_64_emit_ctx *emit_ctx, const cg_x86_64 *code) {
  strbuf *buffer = strbuf_new(0, 0);

  cg_ctx ctx;
  cg_ctx_init(&ctx, buffer, emit_ctx->exceptions);

  cg_emit_section(&ctx, ".data", code->data);
  cg_emit_section(&ctx, ".text", code->text);
  cg_emit_section(&ctx, ".u_debug_info", code->debug_info);
  cg_emit_section(&ctx, ".u_debug_line", code->debug_line);
  cg_emit_section(&ctx, ".u_debug_str", code->debug_str);

  cg_ctx_deinit(&ctx);

  return strbuf_detach(buffer);
}
