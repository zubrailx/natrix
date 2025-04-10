#include "x86_64.h"
#include "util/log.h"
#include "util/macro.h"

static void cg_x86_64_symbol_free(cg_x86_64_symbol *self);
static void cg_x86_64_data_free(cg_x86_64_data *self);
static void cg_x86_64_text_free(cg_x86_64_text *self);

static inline void cg_x86_64_unit_init(cg_x86_64_unit     *base,
                                       cg_x86_64_unit_kind kind) {
  base->kind = kind;
}

static inline void cg_x86_64_unit_deinit(cg_x86_64_unit *base) { UNUSED(base); }

void cg_x86_64_unit_free(cg_x86_64_unit *generic) {
  if (!generic) {
    return;
  }

  switch (generic->kind) {
    case CG_X86_64_UNIT_DATA:
      return cg_x86_64_data_free((cg_x86_64_data *)generic);
    case CG_X86_64_UNIT_TEXT:
      return cg_x86_64_text_free((cg_x86_64_text *)generic);
    case CG_X86_64_UNIT_SYMBOL:
      return cg_x86_64_symbol_free((cg_x86_64_symbol *)generic);
  }
  error("unexpected assembly unit %d %p", generic->kind, generic);
}

cg_x86_64_symbol *cg_x86_64_symbol_new_data(char *name) {
  cg_x86_64_symbol *self = MALLOC(cg_x86_64_symbol);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_SYMBOL);
  self->kind = CG_X86_64_SYMBOL_DATA;
  self->name = name;
  return self;
}

cg_x86_64_symbol *cg_x86_64_symbol_new_data_ln(char *name) {
  cg_x86_64_symbol *self = MALLOC(cg_x86_64_symbol);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_SYMBOL);
  self->kind = CG_X86_64_SYMBOL_DATA_LN;
  self->name = name;
  return self;
}

cg_x86_64_symbol *cg_x86_64_symbol_new_text(char *name) {
  cg_x86_64_symbol *self = MALLOC(cg_x86_64_symbol);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_SYMBOL);
  self->kind = CG_X86_64_SYMBOL_TEXT;
  self->name = name;
  return self;
}

cg_x86_64_symbol *cg_x86_64_symbol_new_extern(char *name) {
  cg_x86_64_symbol *self = MALLOC(cg_x86_64_symbol);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_SYMBOL);
  self->kind = CG_X86_64_SYMBOL_EXTERN;
  self->name = name;
  return self;
}

cg_x86_64_symbol *cg_x86_64_symbol_new_global(char *name) {
  cg_x86_64_symbol *self = MALLOC(cg_x86_64_symbol);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_SYMBOL);
  self->kind = CG_X86_64_SYMBOL_GLOBAL;
  self->name = name;
  return self;
}

static void cg_x86_64_symbol_free(cg_x86_64_symbol *self) {
  if (self) {
    free(self->name);
    cg_x86_64_unit_deinit(&self->base);
    free(self);
  }
}

cg_x86_64_data *cg_x86_64_data_new_byte(uint8_t data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind      = CG_X86_64_DATA_BYTE;
  self->data_byte = data;
  return self;
}

cg_x86_64_data *cg_x86_64_data_new_word(uint16_t data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind      = CG_X86_64_DATA_WORD;
  self->data_word = data;
  return self;
}

cg_x86_64_data *cg_x86_64_data_new_long(uint32_t data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind      = CG_X86_64_DATA_LONG;
  self->data_long = data;
  return self;
}

cg_x86_64_data *cg_x86_64_data_new_quad(uint64_t data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind      = CG_X86_64_DATA_QUAD;
  self->data_quad = data;
  return self;
}

cg_x86_64_data *cg_x86_64_data_new_ascii(uint8_t *data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind       = CG_X86_64_DATA_ASCII;
  self->data_ascii = data;
  return self;
}

cg_x86_64_data *cg_x86_64_data_new_bytes(size_t len, uint8_t *data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind       = CG_X86_64_DATA_BYTES;
  self->data_len   = len;
  self->data_bytes = data;
  return self;
}

cg_x86_64_data *cg_x86_64_data_new_symbol(char *data) {
  cg_x86_64_data *self = MALLOC(cg_x86_64_data);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_DATA);
  self->kind        = CG_X86_64_DATA_SYMBOL;
  self->data_symbol = data;
  return self;
}

static void cg_x86_64_data_free(cg_x86_64_data *self) {
  if (self) {
    switch (self->kind) {
      case CG_X86_64_DATA_BYTE:
        break;
      case CG_X86_64_DATA_WORD:
        break;
      case CG_X86_64_DATA_LONG:
        break;
      case CG_X86_64_DATA_QUAD:
        break;
      case CG_X86_64_DATA_ASCII:
        free(self->data_ascii);
        break;
      case CG_X86_64_DATA_BYTES:
        free(self->data_bytes);
        break;
      case CG_X86_64_DATA_SYMBOL:
        free(self->data_symbol);
        break;
      default:
        error("unknown data kind %d %p", self->kind, self);
    }
    cg_x86_64_unit_deinit(&self->base);
    free(self);
  }
}

cg_x86_64_op *cg_x86_64_op_new_register(cg_x86_64_reg reg) {
  cg_x86_64_op *self = MALLOC(cg_x86_64_op);
  self->kind         = CG_X86_64_MODE_REGISTER;
  self->reg.reg      = reg;
  return self;
}

cg_x86_64_op *cg_x86_64_op_new_direct(char *sym_addr) {
  cg_x86_64_op *self    = MALLOC(cg_x86_64_op);
  self->kind            = CG_X86_64_MODE_DIRECT;
  self->direct.sym_addr = sym_addr;
  return self;
}

cg_x86_64_op *cg_x86_64_op_new_indexed(char *sym_addr, cg_x86_64_reg reg_index,
                                       uint64_t imm_multi) {
  cg_x86_64_op *self      = MALLOC(cg_x86_64_op);
  self->kind              = CG_X86_64_MODE_INDEXED;
  self->indexed.sym_addr  = sym_addr;
  self->indexed.reg_index = reg_index;
  self->indexed.imm_multi = imm_multi;
  return self;
}

cg_x86_64_op *cg_x86_64_op_new_indirect(cg_x86_64_reg reg_base) {
  cg_x86_64_op *self      = MALLOC(cg_x86_64_op);
  self->kind              = CG_X86_64_MODE_INDIRECT;
  self->indirect.reg_base = reg_base;
  return self;
}

cg_x86_64_op *cg_x86_64_op_new_base_imm(uint64_t      imm_offset,
                                        cg_x86_64_reg reg_base) {
  cg_x86_64_op *self        = MALLOC(cg_x86_64_op);
  self->kind                = CG_X86_64_MODE_BASE_IMM;
  self->base_imm.imm_offset = imm_offset;
  self->base_imm.reg_base   = reg_base;
  return self;
}

cg_x86_64_op *cg_x86_64_op_new_base_sym(char         *sym_addr,
                                        cg_x86_64_reg reg_base) {
  cg_x86_64_op *self      = MALLOC(cg_x86_64_op);
  self->kind              = CG_X86_64_MODE_BASE_SYM;
  self->base_sym.sym_addr = sym_addr;
  self->base_sym.reg_base = reg_base;
  return self;
}

cg_x86_64_op *cg_x86_64_op_new_immediate(uint64_t imm_const) {
  cg_x86_64_op *self  = MALLOC(cg_x86_64_op);
  self->kind          = CG_X86_64_MODE_IMMEDIATE;
  self->imm.imm_const = imm_const;
  return self;
}

void cg_x86_64_op_free(cg_x86_64_op *self) {
  if (self) {
    switch (self->kind) {
      case CG_X86_64_MODE_REGISTER:
        break;
      case CG_X86_64_MODE_DIRECT:
        free(self->direct.sym_addr);
        break;
      case CG_X86_64_MODE_INDEXED:
        free(self->indexed.sym_addr);
        break;
      case CG_X86_64_MODE_INDIRECT:
        break;
      case CG_X86_64_MODE_BASE_IMM:
        break;
      case CG_X86_64_MODE_BASE_SYM:
        free(self->base_sym.sym_addr);
        break;
      case CG_X86_64_MODE_IMMEDIATE:
        break;
        error("unexpected op kind %d %p", self->kind, self);
    }
    free(self);
  }
}

cg_x86_64_size cg_x86_64_mnem_size(cg_x86_64_mnem mnem) {
  switch (mnem) {
    case CG_X86_64_MNEM_PUSHQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_POPQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_MOVB:
      return CG_X86_64_SIZE_BYTE;
    case CG_X86_64_MNEM_MOVL:
      return CG_X86_64_SIZE_LONG;
    case CG_X86_64_MNEM_MOVQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_RETQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_SYSCALL:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_XORQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_SUBQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_ADDQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_LEAQ:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_CALL:
      return CG_X86_64_SIZE_QUAD;
    case CG_X86_64_MNEM_TESTB:
      return CG_X86_64_SIZE_BYTE;
    case CG_X86_64_MNEM_JZ:
      return CG_X86_64_SIZE_UNKNOWN;
    case CG_X86_64_MNEM_JNZ:
      return CG_X86_64_SIZE_UNKNOWN;
    case CG_X86_64_MNEM_JMP:
      return CG_X86_64_SIZE_UNKNOWN;
  }
  error("unexpected mnem %d", mnem);
  return CG_X86_64_SIZE_UNKNOWN;
}

cg_x86_64_text *cg_x86_64_text_new(cg_x86_64_mnem     mnem,
                                   list_cg_x86_64_op *operands) {
  cg_x86_64_text *self = MALLOC(cg_x86_64_text);
  cg_x86_64_unit_init(&self->base, CG_X86_64_UNIT_TEXT);
  self->mnem     = mnem;
  self->operands = operands;
  return self;
}

static void cg_x86_64_text_free(cg_x86_64_text *self) {
  if (self) {
    list_cg_x86_64_op_free(self->operands);
    cg_x86_64_unit_deinit(&self->base);
    free(self);
  }
}

cg_x86_64 *cg_x86_64_new() {
  cg_x86_64 *self  = MALLOC(cg_x86_64);
  self->data       = list_cg_x86_64_unit_new();
  self->text       = list_cg_x86_64_unit_new();
  self->debug_info = list_cg_x86_64_unit_new();
  self->debug_line = list_cg_x86_64_unit_new();
  self->debug_str  = list_cg_x86_64_unit_new();
  return self;
}

void cg_x86_64_free(cg_x86_64 *self) {
  if (!self) {
    return;
  }
  list_cg_x86_64_unit_free(self->data);
  list_cg_x86_64_unit_free(self->text);
  list_cg_x86_64_unit_free(self->debug_info);
  list_cg_x86_64_unit_free(self->debug_line);
  list_cg_x86_64_unit_free(self->debug_str);
  free(self);
}
