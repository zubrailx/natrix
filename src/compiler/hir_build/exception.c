#include "exception.h"

void hir_exception_add_error(list_exception       *exceptions,
                             exception_subtype_hir subtype, const span *span,
                             const char *format, ...) {
  va_list args;
  va_start(args, format);
  hir_exception_add_error_v(exceptions, subtype, span, format, args);
  va_end(args);
}

void hir_exception_add_error_v(list_exception       *exceptions,
                               exception_subtype_hir subtype, const span *span,
                               const char *format, va_list args) {
  exception *exc =
      exception_new_v(EXCEPTION_LEVEL_ERROR, EXCEPTION_HIR, subtype,
                      span->source_ref, span->line_start, span->pos_start,
                      exception_subtype_hir_str(subtype), format, args);
  list_exception_push_back(exceptions, exc);
}
