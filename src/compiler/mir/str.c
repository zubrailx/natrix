#include "str.h"
#include "compiler/type_table/str.h"
#include "util/log.h"
#include "util/strbuf.h"
#include <string.h>

char *mir_lit_value_str(const mir_lit *lit) {
  if (!lit) {
    return NULL;
  }

  const type_base *type = lit->type_ref->type;

  switch (type->kind) {
    case TYPE_PRIMITIVE: {
      char            buf[64];
      strbuf         *buffer = strbuf_new(64, 0);
      type_primitive *self   = (typeof(self))type;
      switch (self->type) {
        case TYPE_PRIMITIVE_BOOL:
          if (lit->value.v_bool) {
            strbuf_append(buffer, "true");
          } else {
            strbuf_append(buffer, "false");
          }
          strbuf_append_f(buffer, buf, " (0x%hhx)", lit->value.v_bool);
          break;
        case TYPE_PRIMITIVE_BYTE:
          strbuf_append_f(buffer, buf, "0x%hhx", lit->value.v_byte);
          break;
        case TYPE_PRIMITIVE_INT:
          strbuf_append_f(buffer, buf, "%d", lit->value.v_int);
          break;
        case TYPE_PRIMITIVE_UINT:
          strbuf_append_f(buffer, buf, "%u", lit->value.v_uint);
          break;
        case TYPE_PRIMITIVE_LONG:
          strbuf_append_f(buffer, buf, "%ld", lit->value.v_long);
          break;
        case TYPE_PRIMITIVE_ULONG:
          strbuf_append_f(buffer, buf, "%lu", lit->value.v_ulong);
          break;
        case TYPE_PRIMITIVE_CHAR:
          strbuf_append(buffer, "'");
          strbuf_append_f(buffer, buf, "%c", lit->value.v_char);
          strbuf_append(buffer, "'");
          break;
        case TYPE_PRIMITIVE_STRING:
          strbuf_append(buffer, "\"");
          strbuf_append(buffer, (const char *)lit->value.v_str);
          strbuf_append(buffer, "\"");
          break;
        case TYPE_PRIMITIVE_VOID:
          strbuf_append(buffer, "()");
          break;
        default:
          warn("unexpected primitive kind %d %p", self->type, lit);
          strbuf_append(buffer, "<unknown>");
          break;
      }
      return strbuf_detach(buffer);
    }
    case TYPE_ARRAY:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME:
    case TYPE_MONO: {
      strbuf *buffer = strbuf_new(64, 0);
      char   *type_s = type_str(type);

      strbuf_append(buffer, "<");
      if (type_s) {
        strbuf_append(buffer, type_s);
        free(type_s);
      } else {
        strbuf_append(buffer, "unknown");
      }
      strbuf_append(buffer, ">");

      return strbuf_detach(buffer);
    }
  }
  warn("unexpected type kind %d %p", type->kind, lit);
  return NULL;
}

const char *mir_stmt_enum_str(mir_stmt_enum kind) {
  switch (kind) {
    case MIR_STMT_OP:
      return "op";
    case MIR_STMT_CALL:
      return "call";
    case MIR_STMT_MEMBER:
      return "member";
    case MIR_STMT_MEMBER_REF:
      return "member_ref";
    case MIR_STMT_BUILTIN:
      return "builtin";
    case MIR_STMT_ASSIGN:
      return "assign";
  }
  warn("unexpected stmt kind %d", kind);
  return "unknown";
}

const char *mir_stmt_builtin_enum_str(mir_stmt_builtin_enum kind) {
  switch (kind) {
    case MIR_STMT_BUILTIN_CAST:
      return "cast";
    case MIR_STMT_BUILTIN_MAKE:
      return "make";
    case MIR_STMT_BUILTIN_PRINT:
      return "print";
    case MIR_STMT_BUILTIN_TYPE:
      return "type";
  }
  warn("unexpected stmt builtin kind %d", kind);
  return "unknown";
}

const char *mir_stmt_op_enum_str(mir_stmt_op_enum kind) {
  switch (kind) {
    case MIR_STMT_OP_UNARY_PLUS:
      return "unary_plus";
    case MIR_STMT_OP_UNARY_MINUS:
      return "unary_minus";
    case MIR_STMT_OP_UNARY_LOGICAL_NOT:
      return "unary_logical_not";
    case MIR_STMT_OP_UNARY_BITWISE_NOT:
      return "unary_bitwise_not";
    case MIR_STMT_OP_UNARY_INC:
      return "unary_inc";
    case MIR_STMT_OP_UNARY_DEC:
      return "unary_dec";
    case MIR_STMT_OP_BINARY_LOGICAL_OR:
      return "binary_logical_not";
    case MIR_STMT_OP_BINARY_LOGICAL_AND:
      return "binary_logical_and";
    case MIR_STMT_OP_BINARY_BITWISE_OR:
      return "binary_bitwise_or";
    case MIR_STMT_OP_BINARY_BITWISE_XOR:
      return "binary_bitwise_xor";
    case MIR_STMT_OP_BINARY_BITWISE_AND:
      return "binary_bitwise_and";
    case MIR_STMT_OP_BINARY_EQUALS:
      return "binary_equals";
    case MIR_STMT_OP_BINARY_NOT_EQUALS:
      return "binary_not_equals";
    case MIR_STMT_OP_BINARY_LESS:
      return "binary_less";
    case MIR_STMT_OP_BINARY_LESS_EQUALS:
      return "binary_less_equals";
    case MIR_STMT_OP_BINARY_GREATER:
      return "binary_greater";
    case MIR_STMT_OP_BINARY_GREATER_EQUALS:
      return "binary_grater_equals";
    case MIR_STMT_OP_BINARY_BITWISE_SHIFT_LEFT:
      return "binary_bitwise_shift_left";
    case MIR_STMT_OP_BINARY_BITWISE_SHIFT_RIGHT:
      return "binary_bitwise_shift_right";
    case MIR_STMT_OP_BINARY_ADD:
      return "binary_add";
    case MIR_STMT_OP_BINARY_SUB:
      return "binary_sub";
    case MIR_STMT_OP_BINARY_MUL:
      return "binary_mul";
    case MIR_STMT_OP_BINARY_DIV:
      return "binary_div";
    case MIR_STMT_OP_BINARY_REM:
      return "binary_rem";
    case MIR_STMT_OP_CALL:
      return "call";
    case MIR_STMT_OP_INDEX:
      return "index";
    case MIR_STMT_OP_INDEX_REF:
      return "index_ref";
    case MIR_STMT_OP_DEREF:
      return "deref";
  }
  warn("unexpected stmt_op kind %d", kind);
  return "unknown";
}

const char *mir_subroutine_enum_str(mir_subroutine_enum kind) {
  switch (kind) {
    case MIR_SUBROUTINE_DEFINED:
      return "DEFINED";
    case MIR_SUBROUTINE_DECLARED:
      return "DECLARED";
    case MIR_SUBROUTINE_IMPORTED:
      return "IMPORTED";
  }
  warn("unexpected subroutine kind %d", kind);
  return "UNKNOWN";
}

const char *mir_subroutine_spec_str(mir_subroutine_spec spec) {
  switch (spec) {
    case MIR_SUBROUTINE_SPEC_EMPTY:
      return "NATIVE";
    case MIR_SUBROUTINE_SPEC_EXTERN:
      return "EXTERN";
  }
  warn("unexpected subroutine spec %d", spec);
  return "UNKNOWN";
}
