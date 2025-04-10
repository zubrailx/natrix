#pragma once

#include <stdarg.h>
#include <stdint.h>

typedef enum exception_type_enum {
  EXCEPTION_UNKNOWN,
  EXCEPTION_LEXER,
  EXCEPTION_PARSER,
  EXCEPTION_TREE,
  EXCEPTION_HIR,
  EXCEPTION_MIR,
  EXCEPTION_CG,
} exception_type;

typedef enum exception_level_enum {
  EXCEPTION_LEVEL_UNKNOWN,
  EXCEPTION_LEVEL_INFO,
  EXCEPTION_LEVEL_WARNING,
  EXCEPTION_LEVEL_ERROR,
} exception_level;

typedef enum exception_subtype_lexer_enum {
  EXCEPTION_LEXER_UNKNOWN,
  EXCEPTION_LEXER_MATCHING,
} exception_subtype_lexer;

typedef enum exception_subtype_parser_enum {
  EXCEPTION_PARSER_UNKNOWN,
  EXCEPTION_PARSER_EOF,
  EXCEPTION_PARSER_GENERIC,
} exception_subtype_parser;

typedef enum exception_subtype_tree_enum {
  EXCEPTION_TREE_UNKNOWN,
  EXCEPTION_TREE_UNEXPECTED_NODE,
} exception_subtype_tree;

typedef enum exception_subtype_hir_enum {
  EXCEPTION_HIR_UNKNOWN,
  EXCEPTION_HIR_HEX_VALIDATION,
  EXCEPTION_HIR_BITS_VALIDATION,
  EXCEPTION_HIR_DEC_VALIDATION,
  EXCEPTION_HIR_BOOL_VALIDATION,
  EXCEPTION_HIR_SYMBOL_REDEFINITION,
  EXCEPTION_HIR_SYMBOL_UNDEFINED,
  EXCEPTION_HIR_TYPE_REDEFINITION,
  EXCEPTION_HIR_TYPE_UNDEFINED,
  EXCEPTION_HIR_TYPE_UNSUPPORTED,
  EXCEPTION_HIR_TYPE_UNEXPECTED,
  EXCEPTION_HIR_TYPE_UNRESOLVED,
  EXCEPTION_HIR_SUBROUTINE_DECL_INVALID,
  EXCEPTION_HIR_STATE_ILLEGAL,
  EXCEPTION_HIR_TEMPLATE_EXPANSION,
} exception_subtype_hir;

typedef enum exception_subtype_mir_enum {
  EXCEPTION_MIR_UNKNOWN,
  EXCEPTION_MIR_NO_RETURN,
  EXCEPTION_MIR_UNEXPECTED_TYPE,
  EXCEPTION_MIR_UNEXPECTED_BREAK,
  EXCEPTION_MIR_UNEXPECTED_RVALUE,
  EXCEPTION_MIR_UNEXPECTED_IDENTIFIER,
  EXCEPTION_MIR_UNEXPECTED_MEMBER,
  EXCEPTION_MIR_SYMBOL_NAME_COLLISION,
} exception_subtype_mir;

typedef enum enception_subtype_cg_enum {
  EXCEPTION_CG_UNKNOWN,
  EXCEPTION_CG_UNEXPECTED_EMIT_FORMAT,
  EXCEPTION_CG_UNSUPPORTED_IMPORTED,
  EXCEPTION_CG_UNEXPECTED_EXTERN_DEFINITION,
  EXCEPTION_CG_UNEXPECTED_RETURN_TYPE,
  EXCEPTION_CG_UNEXPECTED_ARGS,
  EXCEPTION_CG_EXTERN_SUBROUTINE_ASSIGNMENT,
  EXCEPTION_CG_UNEXPECTED_TYPE,
  EXCEPTION_CG_UNSUPPORTED_EXTERN_TYPE,
  EXCEPTION_CG_UNSUPPORTED_CLASS_DECLARED,
  EXCEPTION_CG_UNSUPPORTED_CLASS_IMPORTED,
} exception_subtype_cg;

typedef struct exception_struct {
  exception_level level;
  exception_type  type;
  int             subtype;
  char           *stream;
  uint32_t        line;
  uint32_t        offset;
  char           *name;
  char           *message;
} exception;

typedef struct exception_helper_struct {
  exception_level level;
  exception_type  type;
  const char     *name;
  int             subtype;
  uint32_t        line;
  uint32_t        offset;
  const char     *stream;
} exception_helper;

exception *exception_new(exception_level level, exception_type type,
                         int subtype, const char *stream, uint32_t line,
                         uint32_t offset, const char *name,
                         const char *message);
exception *exception_new_v(exception_level level, exception_type type,
                           int subtype, const char *stream, uint32_t line,
                           uint32_t offset, const char *name,
                           const char *format, va_list args);
exception *exception_new_f(exception_level level, exception_type type,
                           int subtype, const char *stream, uint32_t line,
                           uint32_t offset, const char *name,
                           const char *format, ...);
exception *exception_new_from_helper(const exception_helper *helper,
                                     char                   *format, ...);
void       exception_free(exception *exc);
char      *exception_str(const exception *exc, int is_color);

const char *exception_subtype_hir_str(exception_subtype_hir ir);
const char *exception_subtype_mir_str(exception_subtype_mir ir);
const char *exception_subtype_cg_str(exception_subtype_cg ir);
