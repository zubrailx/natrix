#include "symbol_table.h"

#include "util/macro.h"
#include <stdlib.h>

symbol_entry *symbol_entry_new(char *id, type_entry *type_ref, span *span) {
  symbol_entry *self = MALLOC(symbol_entry);
  self->name         = id;
  self->type_ref     = type_ref;
  self->span         = span;
  return self;
}

void symbol_entry_free(symbol_entry *self) {
  if (self) {
    free(self->name);
    self->type_ref = NULL;
    span_free(self->span);
    free(self);
  }
}

symbol_table *symbol_table_new() {
  symbol_table *self = MALLOC(symbol_table);
  self->entries      = list_symbol_entry_new();
  return self;
}

void symbol_table_free(symbol_table *self) {
  if (self) {
    list_symbol_entry_free(self->entries);
    free(self);
  }
}

symbol_entry *symbol_table_emplace(symbol_table *self, char *id,
                                   type_entry *type_ref, span *span) {
  symbol_entry *entry = symbol_entry_new(id, type_ref, span);
  list_symbol_entry_push_back(self->entries, entry);
  return entry;
}
