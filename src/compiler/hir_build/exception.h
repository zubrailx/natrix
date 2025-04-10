#pragma once

#include "compiler/exception/list.h"
#include "compiler/span/span.h"

void hir_exception_add_error(list_exception       *exceptions,
                             exception_subtype_hir subtype, const span *span,
                             const char *format, ...);

void hir_exception_add_error_v(list_exception       *exceptions,
                               exception_subtype_hir subtype, const span *span,
                               const char *format, va_list args);
