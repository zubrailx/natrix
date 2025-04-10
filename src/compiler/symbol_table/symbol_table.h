#pragma once

#include "compiler/span/span.h"
#include "compiler/type_table/type_table.h"

typedef struct symbol_entry_struct {
  char       *name; // used inside scope resolution
  type_entry *type_ref;
  span       *span;
} symbol_entry;

symbol_entry *symbol_entry_new(char *id, type_entry *type_ref, span *span);
void          symbol_entry_free(symbol_entry *self);

static inline void container_delete_symbol_entry(void *lsv) {
  symbol_entry_free(lsv);
}
LIST_DECLARE_STATIC_INLINE(list_symbol_entry, symbol_entry, container_cmp_false,
                           container_new_move, container_delete_symbol_entry);

typedef struct symbol_table_struct {
  list_symbol_entry *entries;
} symbol_table;

symbol_table *symbol_table_new();
void          symbol_table_free(symbol_table *self);

symbol_entry *symbol_table_emplace(symbol_table *self, char *id,
                                   type_entry *type_ref, span *span);
