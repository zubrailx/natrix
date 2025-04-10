#include "exception.h"

void cg_exception_add_error(list_exception      *exceptions,
                            exception_subtype_cg subtype, const span *span,
                            const char *format, ...) {
  va_list args;
  va_start(args, format);
  cg_exception_add_error_v(exceptions, subtype, span, format, args);
  va_end(args);
}

void cg_exception_add_error_v(list_exception      *exceptions,
                              exception_subtype_cg subtype, const span *span,
                              const char *format, va_list args) {
  exception *exc =
      exception_new_v(EXCEPTION_LEVEL_ERROR, EXCEPTION_CG, subtype,
                      span ? span->source_ref : NULL,
                      span ? span->line_start : 0, span ? span->pos_start : 0,
                      exception_subtype_cg_str(subtype), format, args);
  list_exception_push_back(exceptions, exc);
}

void cg_exception_add_warning(list_exception      *exceptions,
                              exception_subtype_cg subtype, const span *span,
                              const char *format, ...) {
  va_list args;
  va_start(args, format);
  cg_exception_add_warning_v(exceptions, subtype, span, format, args);
  va_end(args);
}

void cg_exception_add_warning_v(list_exception      *exceptions,
                                exception_subtype_cg subtype, const span *span,
                                const char *format, va_list args) {
  exception *exc =
      exception_new_v(EXCEPTION_LEVEL_WARNING, EXCEPTION_CG, subtype,
                      span ? span->source_ref : NULL,
                      span ? span->line_start : 0, span ? span->pos_start : 0,
                      exception_subtype_cg_str(subtype), format, args);
  list_exception_push_back(exceptions, exc);
}
