#include "str.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/math.h"
#include <stdio.h>
#include <string.h>

const char *hir_expr_unary_enum_str(hir_expr_unary_enum type) {
  switch (type) {
    case HIR_EXPR_UNARY_PLUS:
      return "PLUS";
    case HIR_EXPR_UNARY_MINUS:
      return "MINUS";
    case HIR_EXPR_UNARY_LOGICAL_NOT:
      return "LOGICAL_NOT";
    case HIR_EXPR_UNARY_BITWISE_NOT:
      return "BITWISE_NOT";
    case HIR_EXPR_UNARY_INC:
      return "INC";
    case HIR_EXPR_UNARY_DEC:
      return "DEC";
  }
  warn("unknown expr_unary_enum %d", type);
  return "UNKNOWN";
}

const char *hir_expr_binary_enum_str(hir_expr_binary_enum type) {
  switch (type) {
    case HIR_EXPR_BINARY_ASSIGN:
      return "ASSIGN";
    case HIR_EXPR_BINARY_LOGICAL_OR:
      return "LOGICAL_OR";
    case HIR_EXPR_BINARY_LOGICAL_AND:
      return "LOGICAL_AND";
    case HIR_EXPR_BINARY_BITWISE_OR:
      return "BITWISE_OR";
    case HIR_EXPR_BINARY_BITWISE_XOR:
      return "BITWISE_XOR";
    case HIR_EXPR_BINARY_BITWISE_AND:
      return "BITWISE_AND";
    case HIR_EXPR_BINARY_EQUALS:
      return "EQUALS";
    case HIR_EXPR_BINARY_NOT_EQUALS:
      return "NOT_EQUALS";
    case HIR_EXPR_BINARY_LESS:
      return "LESS";
    case HIR_EXPR_BINARY_LESS_EQUALS:
      return "LESS_EQUALS";
    case HIR_EXPR_BINARY_GREATER:
      return "GREATER";
    case HIR_EXPR_BINARY_GREATER_EQUALS:
      return "GREATER_EQUALS";
    case HIR_EXPR_BINARY_BITWISE_SHIFT_LEFT:
      return "BITWISE_SHIFT_LEFT";
    case HIR_EXPR_BINARY_BITWISE_SHIFT_RIGHT:
      return "BITWISE_SHIFT_RIGHT";
    case HIR_EXPR_BINARY_ADD:
      return "ADD";
    case HIR_EXPR_BINARY_SUB:
      return "SUB";
    case HIR_EXPR_BINARY_MUL:
      return "MUL";
    case HIR_EXPR_BINARY_DIV:
      return "DIV";
    case HIR_EXPR_BINARY_REM:
      return "REM";
    case HIR_EXPR_BINARY_MEMBER:
      return "MEMBER";
  }
  warn("unknown expr_binary_enum %d", type);
  return "UNKNOWN";
}

const char *hir_node_enum_str(hir_node_enum kind) {
  switch (kind) {
    case HIR_NODE_ID:
      return "ID";
    case HIR_NODE_LIT:
      return "LIT";
    case HIR_NODE_PARAM:
      return "PARAM";
    case HIR_NODE_VAR:
      return "VAR";
    case HIR_NODE_SUBROUTINE:
      return "SUBROUTINE";
    case HIR_NODE_STMT:
      return "STMT";
    case HIR_NODE_EXPR:
      return "EXPR";
    case HIR_NODE_CLASS:
      return "CLASS";
    case HIR_NODE_METHOD:
      return "METHOD";
  }
  warn("unknown hir_node_enum %d", kind);
  return "UNKNOWN";
}

const char *hir_stmt_enum_str(hir_stmt_enum kind) {
  switch (kind) {
    case HIR_STMT_IF:
      return "IF";
    case HIR_STMT_BLOCK:
      return "BLOCK";
    case HIR_STMT_WHILE:
      return "WHILE";
    case HIR_STMT_DO:
      return "DO";
    case HIR_STMT_BREAK:
      return "BREAK";
    case HIR_STMT_EXPR:
      return "EXPR";
    case HIR_STMT_RETURN:
      return "RETURN";
  }
  warn("unknown stmt enum %d", kind);
  return "UNKNOWN";
}

const char *hir_expr_enum_str(hir_expr_enum kind) {
  switch (kind) {
    case HIR_EXPR_UNARY:
      return "UNARY";
    case HIR_EXPR_BINARY:
      return "BINARY";
    case HIR_EXPR_LITERAL:
      return "LITERAL";
    case HIR_EXPR_IDENTIFIER:
      return "IDENTIFIER";
    case HIR_EXPR_CALL:
      return "CALL";
    case HIR_EXPR_INDEX:
      return "INDEX";
    case HIR_EXPR_BUILTIN:
      return "BUILTIN";
  }
  warn("unknown expr enum %d", kind);
  return "UNKNOWN";
}

const char *hir_expr_builtin_enum_str(hir_expr_builtin_enum kind) {
  switch (kind) {
    case HIR_EXPR_BUILTIN_CAST:
      return "CAST";
    case HIR_EXPR_BUILTIN_MAKE:
      return "MAKE";
    case HIR_EXPR_BUILTIN_PRINT:
      return "PRINT";
    case HIR_EXPR_BUILTIN_TYPE:
      return "TYPE";
  }
  warn("unknown expr builtin enum %d", kind);
  return "UNKNOWN";
}

char *hir_lit_value_str(const hir_lit *lit) {
  hir_type_enum type;
  int           type_expected = 1;

  if (lit->state & HIR_STATE_BIND_TYPE) {
    switch (lit->type_ref->type->kind) {
      case TYPE_PRIMITIVE: {
        const type_primitive *self = (type_primitive *)lit->type_ref->type;
        switch (self->type) {
          case TYPE_PRIMITIVE_BOOL:
            type = HIR_TYPE_BOOL;
            break;
          case TYPE_PRIMITIVE_BYTE:
            type = HIR_TYPE_BYTE;
            break;
          case TYPE_PRIMITIVE_INT:
            type = HIR_TYPE_INT;
            break;
          case TYPE_PRIMITIVE_UINT:
            type = HIR_TYPE_UINT;
            break;
          case TYPE_PRIMITIVE_LONG:
            type = HIR_TYPE_LONG;
            break;
          case TYPE_PRIMITIVE_ULONG:
            type = HIR_TYPE_ULONG;
            break;
          case TYPE_PRIMITIVE_CHAR:
            type = HIR_TYPE_CHAR;
            break;
          case TYPE_PRIMITIVE_STRING:
            type = HIR_TYPE_STRING;
            break;
          case TYPE_PRIMITIVE_VOID:
            type = HIR_TYPE_VOID;
            break;
          default:
            type_expected = 0;
            break;
        }
      } break;
      case TYPE_ARRAY:
      case TYPE_CALLABLE:
      case TYPE_CLASS_T:
      case TYPE_TYPENAME:
      case TYPE_MONO:
      default:
        type_expected = 0;
        break;
    }
  } else {
    type = lit->type_hir->type;
  }

  if (type_expected) {
    switch (type) {
      case HIR_TYPE_STRING:
        return strdup(lit->value.v_str);
      case HIR_TYPE_CHAR:
        return strdup(lit->value.v_char);
      case HIR_TYPE_BOOL: {
        if (lit->value.v_bool) {
          return strdup("true");
        } else {
          return strdup("false");
        }
      } break;
      case HIR_TYPE_VOID:
        return strdup("()");
      case HIR_TYPE_BYTE:
      case HIR_TYPE_INT:
      case HIR_TYPE_LONG: {
        char *buf = MALLOCN(char, 32);
        snprintf(buf, 31, "%ld", lit->value.v_long);
        return buf;
      }
      case HIR_TYPE_UINT:
      case HIR_TYPE_ULONG: {
        char *buf = MALLOCN(char, 32);
        snprintf(buf, 31, "%lu", lit->value.v_ulong);
        return buf;
      }
      default:
        break;
    }
  }
  warn("unexpected lit type %d", type);
  return strdup("(unknown)");
}

const char *hir_method_modifier_enum_str(hir_method_modifier_enum kind) {
  switch (kind) {
    case HIR_METHOD_MODIFIER_ENUM_EMPTY:
      return "EMPTY";
    case HIR_METHOD_MODIFIER_ENUM_PUBLIC:
      return "PUBLIC";
    case HIR_METHOD_MODIFIER_ENUM_PRIVATE:
      return "PRIVATE";
  }
  warn("unexpected method modifier kind %d", kind);
  return "UNKNOWN";
}

const char *hir_state_enum_str(hir_state_enum state) {
  switch (state) {
    case HIR_STATE_INITIAL:
      return "INITIAL";
    case HIR_STATE_BIND_TYPE:
      return "BIND_TYPE";
    case HIR_STATE_BIND_SYMBOL:
      return "BIND_SYMBOL";
      break;
    case HIR_STATE_BIND_SYMBOL_AND_TYPE:
      return "BIND_SYMBOL | BIND_TYPE";
  }
  warn("unexpected hir state %d", state);
  return "UNKNOWN";
}

const char *hir_subroutine_spec_str(hir_subroutine_spec spec) {
  switch (spec) {
    case HIR_SUBROUTINE_SPEC_EMPTY:
      return "EMPTY";
    case HIR_SUBROUTINE_SPEC_EXTERN:
      return "EXTERN";
  }
  warn("unexpected subroutine spec %d", spec);
  return "UNKNOWN";
}
