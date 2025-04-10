#pragma once

#include <stddef.h>

typedef struct span_struct {
  const char *source_ref;
  size_t      line_start;
  size_t      line_end;
  size_t      pos_start;
  size_t      pos_end;
} span;

span  span_make(const char *source, size_t line_start, size_t line_end,
                size_t pos_start, size_t pos_end);
span *span_new(const char *source, size_t line_start, size_t line_end,
               size_t pos_start, size_t pos_end);
void  span_free(span *self);
span *span_copy(const span *self);
