#include "str.h"

#include "compiler/span/str.h"
#include "compiler/type_table/str.h"
#include "compiler/type_table/type_table.h"
#include "util/macro.h"
#include "util/math.h"
#include "util/strtable.h"
#include <stdio.h>
#include <string.h>

LIST_DECLARE_STATIC_INLINE(list_chars, char, container_cmp_false,
                           container_new_chars, container_delete_chars);

static inline void container_delete_list_chars(void *data) {
  list_chars_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_list_chars, list_chars, container_cmp_false,
                           container_new_move, container_delete_list_chars);

char *symbol_table_str(const symbol_table *table, const char *title) {
  if (!table) {
    return NULL;
  }

  list_chars *header = list_chars_new();
  list_chars_push_back(header, "PTR");
  list_chars_push_back(header, "NAME");
  list_chars_push_back(header, "TYPE_ID");
  list_chars_push_back(header, "TYPE");
  list_chars_push_back(header, "SPAN");

  list_list_chars *body = list_list_chars_new();
  for (list_symbol_entry_it it = list_symbol_entry_begin(table->entries);
       !END(it); NEXT(it)) {
    list_chars         *row = list_chars_new();
    char                buf[64];
    const symbol_entry *entry = GET(it);
    // ID
    snprintf(buf, STRMAXLEN(buf), "%p", entry);
    list_chars_push_back(row, buf);
    // NAME
    list_chars_push_back(row, entry->name);
    // TYPE_ID
    if (entry->type_ref) {
      snprintf(buf, STRMAXLEN(buf), "%p", entry->type_ref);
    } else {
      snprintf(buf, STRMAXLEN(buf), "-");
    }
    list_chars_push_back(row, buf);
    // TYPE
    char *type_s = type_str(entry->type_ref ? entry->type_ref->type : NULL);
    if (type_s) {
      list_chars_push_back(row, type_s);
      free(type_s);
    } else {
      list_chars_push_back(row, "(nil)");
    }
    // SPAN
    if (entry->span) {
      char *span_s = span_str(entry->span);
      list_chars_push_back(row, span_s);
      free(span_s);
    } else {
      list_chars_push_back(row, "-");
    }
    list_list_chars_push_back(body, row);
  }

  char *result = strtable_build(title, &header->list, &body->list);

  list_list_chars_free(body);
  list_chars_free(header);

  return result;
}
