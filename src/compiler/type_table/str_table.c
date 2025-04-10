#include "str.h"

#include "compiler/span/str.h"
#include "compiler/type_table/type_table.h"
#include "util/macro.h"
#include "util/math.h"
#include "util/strbuf.h"
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

static void type_table_str_extra_ref(strbuf *buffer, const type_base *self) {
  char buf[64];

  const type_entry *child_entry = self->type_entry_ref;
  char             *type_s      = type_str(self);
  if (type_s) {
    strbuf_append(buffer, type_s);
    snprintf(buf, STRMAXLEN(buf), " -> ID(%p)", child_entry);
    strbuf_append(buffer, buf);
    free(type_s);
  }
}

static char *type_table_str_extra(const type_entry *entry) {
  strbuf *buffer = strbuf_new(64, 0);
  switch (entry->type->kind) {
    case TYPE_ARRAY: {
      const type_array *self = (const type_array *)entry->type;
      type_table_str_extra_ref(buffer, self->element_ref);
    } break;
    case TYPE_CALLABLE: {
      size_t               comma = 0;
      const type_callable *self  = (const type_callable *)entry->type;
      if (self->ret_ref) {
        type_table_str_extra_ref(buffer, self->ret_ref);
        ++comma;
      }

      // print as list
      list_type_ref_it it = list_type_ref_begin(self->params);
      if (!END(it)) {
        if (GET(it)) {
          if (comma) {
            strbuf_append(buffer, ", ");
          }
          type_table_str_extra_ref(buffer, GET(it));
          ++comma;
        }
        for (NEXT(it); !END(it); NEXT(it)) {
          if (GET(it)) {
            if (comma) {
              strbuf_append(buffer, ", ");
            }
            type_table_str_extra_ref(buffer, GET(it));
          }
        }
      }
    } break;
    case TYPE_CLASS_T: {
      const type_class_t *self  = (const type_class_t *)entry->type;
      size_t              comma = 0;

      list_type_ref_it it = list_type_ref_begin(self->parents);
      if (!END(it)) {
        type_table_str_extra_ref(buffer, GET(it));
        ++comma;
        for (NEXT(it); !END(it); NEXT(it)) {
          strbuf_append(buffer, ", ");
          type_table_str_extra_ref(buffer, GET(it));
        }
      }
      it = list_type_ref_begin(self->typenames);
      if (!END(it)) {
        if (comma) {
          strbuf_append(buffer, ", ");
        }
        type_table_str_extra_ref(buffer, GET(it));
        ++comma;
        for (NEXT(it); !END(it); NEXT(it)) {
          strbuf_append(buffer, ", ");
          type_table_str_extra_ref(buffer, GET(it));
        }
      }
    } break;
    case TYPE_TYPENAME: {
      const type_typename *self         = (const type_typename *)entry->type;
      const type_entry    *parent_entry = self->source_ref->type_entry_ref;

      strbuf_append(buffer, "references ");
      type_table_str_extra_ref(buffer, parent_entry->type);
    } break;
    case TYPE_MONO: {
      const type_mono *self = (const type_mono *)entry->type;
      strbuf_append(buffer, "instantiates ");
      type_table_str_extra_ref(buffer, self->type_ref);
      for (list_type_ref_it it = list_type_ref_begin(self->types); !END(it);
           NEXT(it)) {
        strbuf_append(buffer, ", ");
        type_table_str_extra_ref(buffer, GET(it));
      }
    } break;
    default:
      break;
  }
  return strbuf_detach(buffer);
}

char *type_table_str(const type_table *table, const char *title) {
  if (!table) {
    return NULL;
  }

  list_chars *header = list_chars_new();
  list_chars_push_back(header, "PTR");
  list_chars_push_back(header, "TYPE_KIND");
  list_chars_push_back(header, "TYPE_MONO");
  list_chars_push_back(header, "TYPE");
  list_chars_push_back(header, "SPAN");
  list_chars_push_back(header, "EXTRA");

  list_list_chars *body = list_list_chars_new();
  for (list_type_entry_it it = list_type_entry_begin(table->entries); !END(it);
       NEXT(it)) {
    list_chars       *row = list_chars_new();
    char              buf[64];
    const type_entry *entry = GET(it);

    // ID
    snprintf(buf, STRMAXLEN(buf), "%p", entry);
    list_chars_push_back(row, buf);
    // TYPE_KIND
    switch (entry->type->kind) {
      case TYPE_PRIMITIVE:
      case TYPE_ARRAY:
      case TYPE_CALLABLE:
      case TYPE_CLASS_T:
      case TYPE_TYPENAME:
        list_chars_push_back(row, (char *)type_enum_str(entry->type->kind));
        break;
      case TYPE_MONO: {
        strbuf *buffer = strbuf_new(32, 0);
        strbuf_append(buffer, (char *)type_enum_str(entry->type->kind));
        strbuf_append(buffer, "(");
        strbuf_append(buffer, (char *)type_enum_str(
                                  ((type_mono *)entry->type)->type_ref->kind));
        strbuf_append(buffer, ")");
        char *kind_s = strbuf_detach(buffer);
        list_chars_push_back(row, kind_s);
        if (kind_s) {
          free(kind_s);
        }
      } break;
    }
    // TYPE_MONO
    type_mono_enum spec;
    if ((spec = type_get_mono(entry->type)) == TYPE_MONO_UNSET) {
      spec = type_calc_mono(entry->type);
    }
    list_chars_push_back(row, (char *)type_mono_enum_str(spec));
    // TYPE
    char *type_s = type_str(entry->type);
    if (type_s) {
      list_chars_push_back(row, type_s);
      free(type_s);
    } else {
      list_chars_push_back(row, "-");
    }
    // SPAN
    if (entry->span) {
      char *span_s = span_str(entry->span);
      list_chars_push_back(row, span_s);
      free(span_s);
    } else {
      list_chars_push_back(row, "-");
    }
    // EXTRA
    char *extra_s = type_table_str_extra(entry);
    if (extra_s) {
      list_chars_push_back(row, extra_s);
      free(extra_s);
    } else {
      list_chars_push_back(row, "");
    }

    list_list_chars_push_back(body, row);
  }

  char *result = strtable_build(title, &header->list, &body->list);

  list_list_chars_free(body);
  list_chars_free(header);

  return result;
}
