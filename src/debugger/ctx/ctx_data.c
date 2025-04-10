#include "ctx_data.h"

#include "util/macro.h"

ctx_breakpoint *ctx_breakpoint_new(size_t id, uint64_t address,
                                   uint64_t file_id, size_t line) {
  ctx_breakpoint *self = MALLOC(ctx_breakpoint);
  self->id             = id;
  self->data           = 0;
  self->address        = address;
  self->file_id        = file_id;
  self->line           = line;
  self->is_enabled     = 0;
  self->is_placed      = 0;
  return self;
}

void ctx_breakpoint_free(ctx_breakpoint *self) {
  if (self) {
    free(self);
  }
}
