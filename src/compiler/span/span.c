#include "span.h"
#include "util/macro.h"
#include <stdlib.h>

span span_make(const char *source_ref, size_t line_start, size_t line_end,
               size_t pos_start, size_t pos_end) {
  return (span){
      .source_ref = source_ref,
      .line_start = line_start,
      .line_end   = line_end,
      .pos_start  = pos_start,
      .pos_end    = pos_end,
  };
}
span *span_new(const char *source_ref, size_t line_start, size_t line_end,
               size_t pos_start, size_t pos_end) {
  span *self = MALLOC(span);
  *self      = span_make(source_ref, line_start, line_end, pos_start, pos_end);
  return self;
}

void span_free(span *self) {
  if (self) {
    free(self);
  }
}

span *span_copy(const span *self) {
  if (self) {
    span *new_self = MALLOC(span);
    *new_self      = *self;
    return new_self;
  }
  return NULL;
}
