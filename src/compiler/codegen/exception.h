#pragma once

#include "compiler/exception/list.h"
#include "compiler/span/span.h"

void cg_exception_add_error(list_exception      *exceptions,
                            exception_subtype_cg subtype, const span *span,
                            const char *format, ...);

void cg_exception_add_error_v(list_exception      *exceptions,
                              exception_subtype_cg subtype, const span *span,
                              const char *format, va_list args);

void cg_exception_add_warning(list_exception      *exceptions,
                              exception_subtype_cg subtype, const span *span,
                              const char *format, ...);

void cg_exception_add_warning_v(list_exception      *exceptions,
                                exception_subtype_cg subtype, const span *span,
                                const char *format, va_list args);
