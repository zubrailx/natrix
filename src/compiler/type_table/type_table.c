#include "type_table.h"

#include "util/macro.h"
#include <stdlib.h>

type_entry *type_entry_new(type_base *type, span *span) {
  type_entry *self = MALLOC(type_entry);
  self->type       = type;
  self->span       = span;
  return self;
}

void type_entry_free(type_entry *self) {
  if (self) {
    type_free(self->type);
    span_free(self->span);
    free(self);
  }
}

type_table *type_table_new() {
  type_table *self = MALLOC(type_table);
  self->entries    = list_type_entry_new();
  return self;
}

void type_table_free(type_table *self) {
  if (self) {
    list_type_entry_free(self->entries);
    free(self);
  }
}

type_entry *type_table_emplace(type_table *self, type_base *type, span *span) {
  type_entry *entry    = type_entry_new(type, span);
  type->type_entry_ref = entry;
  list_type_entry_push_back(self->entries, entry);
  return entry;
}
