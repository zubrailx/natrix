#include "exception.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/colors.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"

void exception_free(exception *exc) {
  if (exc) {
    if (exc->stream) {
      free(exc->stream);
    }

    if (exc->name) {
      free(exc->name);
    }

    if (exc->message) {
      free(exc->message);
    }

    free(exc);
  }
}

exception *exception_new(exception_level level, exception_type type,
                         int subtype, const char *stream, uint32_t line,
                         uint32_t offset, const char *name,
                         const char *message) {

  exception *exc = calloc(1, sizeof(exception));

  exc->level   = level;
  exc->type    = type;
  exc->subtype = subtype;

  if (stream) {
    exc->stream = strdup(stream);
  }
  exc->line   = line;
  exc->offset = offset;

  if (name) {
    exc->name = strdup(name);
  }

  if (message) {
    exc->message = strdup(message);
  }

  return exc;
};

exception *exception_new_v(exception_level level, exception_type type,
                           int subtype, const char *stream, uint32_t line,
                           uint32_t offset, const char *name,
                           const char *format, va_list args) {
  char message[256]; // message size
  message[0] = '\0';
  vsnprintf(message, STRMAXLEN(message), format, args);
  return exception_new(level, type, subtype, stream, line, offset, name,
                       message);
}

exception *exception_new_f(exception_level level, exception_type type,
                           int subtype, const char *stream, uint32_t line,
                           uint32_t offset, const char *name,
                           const char *format, ...) {
  exception *exc;
  va_list    args;
  va_start(args, format);
  exc = exception_new_v(level, type, subtype, stream, line, offset, name,
                        format, args);
  va_end(args);
  return exc;
}

char *exception_str(const exception *exc, int is_color) {
  char buf[256];

  char *exc_stream  = "<empty>";
  char *exc_name    = "<empty>";
  char *exc_message = "<empty>";

  {
    if (exc->stream) {
      exc_stream = exc->stream;
    }

    if (exc->name) {
      exc_name = exc->name;
    }

    if (exc->message) {
      exc_message = exc->message;
    }
  }

  strbuf *buffer = strbuf_new(sizeof(buf), 0);

  switch (exc->level) {
    case EXCEPTION_LEVEL_INFO:
      if (is_color) {
        strbuf_append(buffer, ANSI_COLOR_CYAN "info" ANSI_COLOR_RESET);
      } else {
        strbuf_append(buffer, "info");
      }
      break;
    case EXCEPTION_LEVEL_WARNING:
      if (is_color) {
        strbuf_append(buffer, ANSI_COLOR_YELLOW "warn" ANSI_COLOR_RESET);
      } else {
        strbuf_append(buffer, "info");
      }
      break;
    case EXCEPTION_LEVEL_ERROR:
      if (is_color) {
        strbuf_append(buffer, ANSI_COLOR_RED "error" ANSI_COLOR_RESET);
      } else {
        strbuf_append(buffer, "info");
      }
      break;
    case EXCEPTION_LEVEL_UNKNOWN:
    default:
      strbuf_append(buffer, "unknown");
      break;
  }

  switch (exc->type) {
    case EXCEPTION_LEXER:
      snprintf(buf, STRMAXLEN(buf), "(L%d): %s\n", exc->subtype, exc_message);
      break;
    case EXCEPTION_PARSER:
      snprintf(buf, STRMAXLEN(buf), "(P%d): %s -> %s", exc->subtype, exc_name,
               exc_message);
      break;
    case EXCEPTION_TREE:
      snprintf(buf, STRMAXLEN(buf), "(T%d): %s -> %s", exc->subtype, exc_name,
               exc_message);
      break;
    case EXCEPTION_HIR:
      snprintf(buf, STRMAXLEN(buf), "(H%d): %s -> %s", exc->subtype, exc_name,
               exc_message);
      break;
    case EXCEPTION_MIR:
      snprintf(buf, STRMAXLEN(buf), "(M%d): %s -> %s", exc->subtype, exc_name,
               exc_message);
      break;
    case EXCEPTION_CG:
      snprintf(buf, STRMAXLEN(buf), "(C%d): %s -> %s", exc->subtype, exc_name,
               exc_message);
      break;
    case EXCEPTION_UNKNOWN:
    default:
      snprintf(buf, STRMAXLEN(buf), "(U%d): %s -> %s", exc->subtype, exc_name,
               exc_message);
      break;
  }
  strbuf_append(buffer, buf);
  strbuf_append(buffer, "\n");

  if (is_color) {
    snprintf(buf, STRMAXLEN(buf),
             ANSI_COLOR_BLUE "  -->" ANSI_COLOR_RESET " %s:%d:%d", exc_stream,
             exc->line, exc->offset);
  } else {
    snprintf(buf, STRMAXLEN(buf), "  --> %s:%d:%d", exc_stream, exc->line,
             exc->offset);
  }

  strbuf_append(buffer, buf);

  return strbuf_detach(buffer);
}

exception *exception_new_from_helper(const exception_helper *helper,
                                     char                   *format, ...) {
  va_list args;
  va_start(args, format);
  exception *exc = exception_new_v(helper->level, helper->type, helper->subtype,
                                   helper->stream, helper->line, helper->offset,
                                   helper->name, format, args);
  va_end(args);
  return exc;
}

const char *exception_subtype_hir_str(exception_subtype_hir type) {
  switch (type) {
    case EXCEPTION_HIR_HEX_VALIDATION:
      return "hex validation";
    case EXCEPTION_HIR_BITS_VALIDATION:
      return "bits validation";
    case EXCEPTION_HIR_DEC_VALIDATION:
      return "dec validation";
    case EXCEPTION_HIR_BOOL_VALIDATION:
      return "bool validation";
    case EXCEPTION_HIR_SYMBOL_REDEFINITION:
      return "symbol redefinition";
    case EXCEPTION_HIR_SYMBOL_UNDEFINED:
      return "symbol undefined";
    case EXCEPTION_HIR_TYPE_REDEFINITION:
      return "symbol redefinition";
    case EXCEPTION_HIR_TYPE_UNDEFINED:
      return "symbol undefined";
    case EXCEPTION_HIR_TYPE_UNSUPPORTED:
      return "type unsupported";
    case EXCEPTION_HIR_SUBROUTINE_DECL_INVALID:
      return "invalid subroutine declaration";
    case EXCEPTION_HIR_TYPE_UNEXPECTED:
      return "type unexpected";
    case EXCEPTION_HIR_TYPE_UNRESOLVED:
      return "type unresolved";
    case EXCEPTION_HIR_STATE_ILLEGAL:
      return "state illegal";
    case EXCEPTION_HIR_TEMPLATE_EXPANSION:
      return "template expansion";
    case EXCEPTION_HIR_UNKNOWN:
      break;
  }
  warn("unknown exception subtype %d", type);
  return "unknown";
}

const char *exception_subtype_mir_str(exception_subtype_mir type) {
  switch (type) {
    case EXCEPTION_MIR_NO_RETURN:
      return "no return";
    case EXCEPTION_MIR_UNEXPECTED_TYPE:
      return "unexpected type";
    case EXCEPTION_MIR_UNEXPECTED_BREAK:
      return "unexpected break";
    case EXCEPTION_MIR_UNEXPECTED_RVALUE:
      return "unexpected rvalue";
    case EXCEPTION_MIR_UNEXPECTED_IDENTIFIER:
      return "unexpected identifeir";
    case EXCEPTION_MIR_UNEXPECTED_MEMBER:
      return "unexpected member";
    case EXCEPTION_MIR_SYMBOL_NAME_COLLISION:
      return "symbol name collision";
    case EXCEPTION_MIR_UNKNOWN:
      break;
  }
  warn("unknown exception subtype %d", type);
  return "unknown";
}

const char *exception_subtype_cg_str(exception_subtype_cg type) {
  switch (type) {
    case EXCEPTION_CG_UNEXPECTED_EMIT_FORMAT:
      return "unexpected emit format";
    case EXCEPTION_CG_UNSUPPORTED_IMPORTED:
      return "unsupported imported subroutine";
    case EXCEPTION_CG_UNEXPECTED_EXTERN_DEFINITION:
      return "unexpected extern definition";
    case EXCEPTION_CG_UNEXPECTED_RETURN_TYPE:
      return "unexpected return type";
    case EXCEPTION_CG_UNEXPECTED_ARGS:
      return "unexpected args";
    case EXCEPTION_CG_EXTERN_SUBROUTINE_ASSIGNMENT:
      return "unexpected extern subroutine assignment";
    case EXCEPTION_CG_UNEXPECTED_TYPE:
      return "unexpected type";
    case EXCEPTION_CG_UNSUPPORTED_EXTERN_TYPE:
      return "unsupported extern type";
    case EXCEPTION_CG_UNSUPPORTED_CLASS_DECLARED:
      return "unsupported class declared subroutine";
    case EXCEPTION_CG_UNSUPPORTED_CLASS_IMPORTED:
      return "unsupported class imported subroutine";
    case EXCEPTION_CG_UNKNOWN:
      break;
  }
  warn("unknown exception subtype %d", type);
  return "unknown";
}
