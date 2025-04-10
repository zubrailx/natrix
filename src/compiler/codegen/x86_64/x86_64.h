#pragma once

#include "util/container_util.h"
#include "util/list.h"

typedef enum cg_x86_64_unit_kind_enum {
  CG_X86_64_UNIT_DATA,
  CG_X86_64_UNIT_TEXT,
  CG_X86_64_UNIT_SYMBOL,
} cg_x86_64_unit_kind;

typedef struct cg_x86_64_unit {
  cg_x86_64_unit_kind kind;
} cg_x86_64_unit;

void cg_x86_64_unit_free(cg_x86_64_unit *generic);

static inline void container_delete_cg_x86_64_unit(void *data) {
  cg_x86_64_unit_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_x86_64_unit, cg_x86_64_unit,
                           container_cmp_false, container_new_move,
                           container_delete_cg_x86_64_unit);

typedef enum cg_x86_64_symbol_kind_enum {
  CG_X86_64_SYMBOL_DATA,
  CG_X86_64_SYMBOL_DATA_LN,
  CG_X86_64_SYMBOL_TEXT,
  CG_X86_64_SYMBOL_EXTERN,
  CG_X86_64_SYMBOL_GLOBAL,
} cg_x86_64_symbol_kind;

typedef struct cg_x86_64_symbol_struct {
  cg_x86_64_unit        base;
  cg_x86_64_symbol_kind kind;
  char                 *name;
} cg_x86_64_symbol;

cg_x86_64_symbol *cg_x86_64_symbol_new_data(char *name);
cg_x86_64_symbol *cg_x86_64_symbol_new_data_ln(char *name);
cg_x86_64_symbol *cg_x86_64_symbol_new_text(char *name);
cg_x86_64_symbol *cg_x86_64_symbol_new_extern(char *name);
cg_x86_64_symbol *cg_x86_64_symbol_new_global(char *name);

typedef enum cg_x86_64_data_kind_enum {
  CG_X86_64_DATA_BYTE,   // 1 byte
  CG_X86_64_DATA_WORD,   // 2 bytes
  CG_X86_64_DATA_LONG,   // 4 bytes
  CG_X86_64_DATA_QUAD,   // 8 bytes
  CG_X86_64_DATA_ASCII,  // ascii string
  CG_X86_64_DATA_BYTES,  // byte array
  CG_X86_64_DATA_SYMBOL, // symbol (quad size)
} cg_x86_64_data_kind;

typedef struct cg_x86_64_data_struct {
  cg_x86_64_unit      base;
  cg_x86_64_data_kind kind;
  size_t              data_len;
  union {
    uint8_t  data_byte;
    uint16_t data_word;
    uint32_t data_long;
    uint64_t data_quad;
    uint8_t *data_ascii;
    uint8_t *data_bytes;
    char    *data_symbol;
  };
} cg_x86_64_data;

cg_x86_64_data *cg_x86_64_data_new_byte(uint8_t data);
cg_x86_64_data *cg_x86_64_data_new_word(uint16_t data);
cg_x86_64_data *cg_x86_64_data_new_long(uint32_t data);
cg_x86_64_data *cg_x86_64_data_new_quad(uint64_t data);
cg_x86_64_data *cg_x86_64_data_new_ascii(uint8_t *data);
cg_x86_64_data *cg_x86_64_data_new_bytes(size_t size, uint8_t *data);
cg_x86_64_data *cg_x86_64_data_new_symbol(char *data);

typedef enum cg_x86_64_reg_enum {
  CG_X86_64_REG_RAX,
  CG_X86_64_REG_RBX,
  CG_X86_64_REG_RCX,
  CG_X86_64_REG_RDX,
  CG_X86_64_REG_RSP,
  CG_X86_64_REG_RBP,
  CG_X86_64_REG_RSI,
  CG_X86_64_REG_RDI,
  CG_X86_64_REG_R8,
  CG_X86_64_REG_R9,
  CG_X86_64_REG_R10,
  CG_X86_64_REG_R11,
  CG_X86_64_REG_R12,
  CG_X86_64_REG_R13,
  CG_X86_64_REG_R14,
  CG_X86_64_REG_R15,
  CG_X86_64_REG_RIP,
} cg_x86_64_reg;

// ----------------------------------------------------------------------------
// MEMORY ADDRESSING:
//
// final address = ADDRESS_OR_OFFSET + %BASE_OR_OFFSET + MULTIPLIER * %INDEX
//               = ADDRESS_OR_OFFSET(%BASE_OR_OFFSET,%INDEX,MULTIPLIER)
//
// direct addressing:
// movl ADDRESS, %eax - load %eax with memory value at ADDRESS
// ADDRESS - constant (without $) or label
//
// indexed:
// movl ADDRESS(,%INDEX,MULTIPLIER), %eax - direct with 'iteration'
//
// indirect (same as base without OFFSET):
// movl (%BASE), %ebx
//
// base pointer (memory at address %BASE + OFFSET):
// movl OFFSET(%BASE) - indirect + OFFSET
//
// ----------------------------------------------------------------------------
// IMMEDIATE:
//
// movl $4, %ebx - load 4 into ebx
// $4 is ADDRESS (constant, 64 bit (hexed))
//
// ----------------------------------------------------------------------------
// REGISTER-TO-REGISTER:
//
// movl %eax, %ebx - register to register
//
// ----------------------------------------------------------------------------

typedef enum x86_64_mode_kind_enum {
  CG_X86_64_MODE_REGISTER,  // reg
  CG_X86_64_MODE_DIRECT,    // symbol
  CG_X86_64_MODE_INDEXED,   // symbol + index + multi
  CG_X86_64_MODE_INDIRECT,  //          base
  CG_X86_64_MODE_BASE_IMM,  // offset + base
  CG_X86_64_MODE_BASE_SYM,  // symbol + base
  CG_X86_64_MODE_IMMEDIATE, // imm
} cg_x86_64_mode_kind;

typedef struct cg_x86_64_op_struct {
  cg_x86_64_mode_kind kind;
  union {
    struct {
      cg_x86_64_reg reg;
    } reg;
    struct {
      char *sym_addr;
    } direct;
    struct {
      char         *sym_addr;
      cg_x86_64_reg reg_index;
      uint64_t      imm_multi;
    } indexed;
    struct {
      cg_x86_64_reg reg_base;
    } indirect;
    struct {
      uint64_t      imm_offset;
      cg_x86_64_reg reg_base;
    } base_imm;
    struct {
      char         *sym_addr;
      cg_x86_64_reg reg_base;
    } base_sym;
    struct {
      uint64_t imm_const;
    } imm;
  };
} cg_x86_64_op;

cg_x86_64_op *cg_x86_64_op_new_register(cg_x86_64_reg reg);
cg_x86_64_op *cg_x86_64_op_new_direct(char *sym_addr);
cg_x86_64_op *cg_x86_64_op_new_indexed(char *sym_addr, cg_x86_64_reg reg_index,
                                       uint64_t imm_multi);
cg_x86_64_op *cg_x86_64_op_new_indirect(cg_x86_64_reg reg_base);
cg_x86_64_op *cg_x86_64_op_new_base_imm(uint64_t      imm_offset,
                                        cg_x86_64_reg reg_base);
cg_x86_64_op *cg_x86_64_op_new_base_sym(char *sym_addr, cg_x86_64_reg reg_base);
cg_x86_64_op *cg_x86_64_op_new_immediate(uint64_t imm_const);

void cg_x86_64_op_free(cg_x86_64_op *self);

static inline void container_delete_cg_x86_64_op(void *data) {
  cg_x86_64_op_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_x86_64_op, cg_x86_64_op, container_cmp_false,
                           container_new_move, container_delete_cg_x86_64_op);

typedef enum cg_x86_64_mnem_enum {
  CG_X86_64_MNEM_PUSHQ,
  CG_X86_64_MNEM_POPQ,
  CG_X86_64_MNEM_MOVB,
  CG_X86_64_MNEM_MOVL,
  CG_X86_64_MNEM_MOVQ,
  CG_X86_64_MNEM_RETQ,
  CG_X86_64_MNEM_SYSCALL,
  CG_X86_64_MNEM_XORQ,
  CG_X86_64_MNEM_SUBQ,
  CG_X86_64_MNEM_ADDQ,
  CG_X86_64_MNEM_LEAQ,
  CG_X86_64_MNEM_CALL,
  CG_X86_64_MNEM_TESTB,
  CG_X86_64_MNEM_JZ,
  CG_X86_64_MNEM_JNZ,
  CG_X86_64_MNEM_JMP,
} cg_x86_64_mnem;

typedef enum cg_x86_64_size_enum {
  CG_X86_64_SIZE_UNKNOWN = 0,
  CG_X86_64_SIZE_BYTE    = 1,
  CG_X86_64_SIZE_WORD    = 2,
  CG_X86_64_SIZE_LONG    = 4,
  CG_X86_64_SIZE_QUAD    = 8,
  CG_X86_64_SIZE_ALIGN   = 16,
} cg_x86_64_size;

cg_x86_64_size cg_x86_64_mnem_size(cg_x86_64_mnem mnem);

// at&t instruction order
typedef struct cg_x86_64_text_struct {
  cg_x86_64_unit     base;
  cg_x86_64_mnem     mnem;
  list_cg_x86_64_op *operands;
} cg_x86_64_text;

cg_x86_64_text *cg_x86_64_text_new(cg_x86_64_mnem     mnem,
                                   list_cg_x86_64_op *operands);

typedef struct cg_x86_64_struct {
  list_cg_x86_64_unit *data;
  list_cg_x86_64_unit *text;
  list_cg_x86_64_unit *debug_info;
  list_cg_x86_64_unit *debug_line;
  list_cg_x86_64_unit *debug_str;
} cg_x86_64;

cg_x86_64 *cg_x86_64_new();
void       cg_x86_64_free(cg_x86_64 *self);
