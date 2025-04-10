#include "exception.h"

void mir_exception_add_error(list_exception       *exceptions,
                             exception_subtype_mir subtype, const span *span,
                             const char *format, ...) {
  va_list args;
  va_start(args, format);
  mir_exception_add_error_v(exceptions, subtype, span, format, args);
  va_end(args);
}

void mir_exception_add_warning(list_exception       *exceptions,
                               exception_subtype_mir subtype, const span *span,
                               const char *format, ...) {
  va_list args;
  va_start(args, format);
  mir_exception_add_warning_v(exceptions, subtype, span, format, args);
  va_end(args);
}

void mir_exception_add_error_v(list_exception       *exceptions,
                               exception_subtype_mir subtype, const span *span,
                               const char *format, va_list args) {
  exception *exc =
      exception_new_v(EXCEPTION_LEVEL_ERROR, EXCEPTION_MIR, subtype,
                      span->source_ref, span->line_start, span->pos_start,
                      exception_subtype_mir_str(subtype), format, args);
  list_exception_push_back(exceptions, exc);
}

void mir_exception_add_warning_v(list_exception       *exceptions,
                                 exception_subtype_mir subtype,
                                 const span *span, const char *format,
                                 va_list args) {
  exception *exc =
      exception_new_v(EXCEPTION_LEVEL_WARNING, EXCEPTION_MIR, subtype,
                      span->source_ref, span->line_start, span->pos_start,
                      exception_subtype_mir_str(subtype), format, args);
  list_exception_push_back(exceptions, exc);
}
