#pragma once

#include "compiler/exception/list.h"
#include "compiler/span/span.h"

void mir_exception_add_error(list_exception       *exceptions,
                             exception_subtype_mir subtype, const span *span,
                             const char *format, ...);
void mir_exception_add_warning(list_exception       *exceptions,
                               exception_subtype_mir subtype, const span *span,
                               const char *format, ...);

void mir_exception_add_error_v(list_exception       *exceptions,
                               exception_subtype_mir subtype, const span *span,
                               const char *format, va_list args);
void mir_exception_add_warning_v(list_exception       *exceptions,
                                 exception_subtype_mir subtype,
                                 const span *span, const char *format,
                                 va_list args);
