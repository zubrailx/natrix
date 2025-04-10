#include "str.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"

static void strbuf_append_type(strbuf *buffer, const hir_type_base *generic);

static void strbuf_append_type_list(strbuf *buffer, const list_hir_type *list,
                                    const char *prefix, const char *suffix) {
  if (list) {
    strbuf_append(buffer, prefix);
    list_hir_type_it it = list_hir_type_begin(list);
    if (!END(it)) {
      strbuf_append_type(buffer, GET(it));
    }
    while (NEXT(it)) {
      strbuf_append(buffer, ", ");
      strbuf_append_type(buffer, GET(it));
    }
    strbuf_append(buffer, suffix);
  }
}

static void strbuf_append_type(strbuf *buffer, const hir_type_base *generic) {
  if (!generic) {
    strbuf_append(buffer, "(nil)");
    return;
  }

  switch (generic->type) {
    case HIR_TYPE_BOOL:
      strbuf_append(buffer, "bool");
      return;
    case HIR_TYPE_BYTE:
      strbuf_append(buffer, "byte");
      return;
    case HIR_TYPE_INT:
      strbuf_append(buffer, "int");
      return;
    case HIR_TYPE_UINT:
      strbuf_append(buffer, "uint");
      return;
    case HIR_TYPE_LONG:
      strbuf_append(buffer, "long");
      return;
    case HIR_TYPE_ULONG:
      strbuf_append(buffer, "ulong");
      return;
    case HIR_TYPE_CHAR:
      strbuf_append(buffer, "char");
      return;
    case HIR_TYPE_STRING:
      strbuf_append(buffer, "string");
      return;
    case HIR_TYPE_VOID:
      strbuf_append(buffer, "void");
      return;
    case HIR_TYPE_ANY:
      strbuf_append(buffer, "any");
      return;
    case HIR_TYPE_CUSTOM: {
      hir_type_custom *custom = (typeof(custom))generic;
      strbuf_append(buffer, custom->name);
      strbuf_append_type_list(buffer, custom->templates, "<", ">");
      return;
    }
    case HIR_TYPE_ARRAY: {
      hir_type_array *array = (typeof(array))generic;
      strbuf_append(buffer, "array[");
      strbuf_append_type(buffer, array->elem_type);
      strbuf_append(buffer, "]");
      return;
    }
  }
  warn("unknown type %d %p", generic->type, generic);
  strbuf_append(buffer, "unknown");
}

const char *hir_type_enum_str(hir_type_enum type) {
  switch (type) {
    case HIR_TYPE_BOOL:
      return "BOOL";
    case HIR_TYPE_BYTE:
      return "BYTE";
    case HIR_TYPE_INT:
      return "INT";
    case HIR_TYPE_UINT:
      return "UINT";
    case HIR_TYPE_LONG:
      return "LONG";
    case HIR_TYPE_ULONG:
      return "ULONG";
    case HIR_TYPE_CHAR:
      return "CHAR";
    case HIR_TYPE_STRING:
      return "STRING";
    case HIR_TYPE_VOID:
      return "VOID";
    case HIR_TYPE_CUSTOM:
      return "CUSTOM";
    case HIR_TYPE_ARRAY:
      return "ARRAY";
    case HIR_TYPE_ANY:
      return "ANY";
  }
  return "UNKNOWN";
}

const char *hir_type_base_str(hir_type_base *self) {
  if (self) {
    return hir_type_enum_str(self->type);
  }
  return "(nil)";
}

char *hir_type_str(const hir_type_base *generic) {
  if (generic) {
    strbuf *buffer = strbuf_new(16, 0);
    strbuf_append_type(buffer, generic);
    return strbuf_detach(buffer);
  }
  return NULL;
}
