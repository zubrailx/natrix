#pragma once

#include "compiler/span/span.h"
#include "compiler/type_table/type.h"

typedef struct type_entry_struct {
  type_base *type;
  span      *span;
} type_entry;

type_entry *type_entry_new(type_base *type, span *span);
void        type_entry_free(type_entry *self);

static inline void container_delete_type_entry(void *lsv) {
  type_entry_free(lsv);
}
LIST_DECLARE_STATIC_INLINE(list_type_entry, type_entry, container_cmp_false,
                           container_new_move, container_delete_type_entry);

// TODO: use hashset instead of list
// compare/hash is required in template expansion
typedef struct type_table_struct {
  list_type_entry *entries;
} type_table;

type_table *type_table_new();
void        type_table_free(type_table *self);

// returns pointer of inserted entry
type_entry *type_table_emplace(type_table *self, type_base *type, span *span);
