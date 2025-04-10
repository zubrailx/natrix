#include "str.h"
#include "util/hashset.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include <string.h>

// PRETTY PRINT TYPE MONO
typedef struct type_map_struct {
  const type_base *from;
  const type_base *to;
} type_map;

static type_map *type_map_new(const type_base *from, const type_base *to) {
  type_map *self = MALLOC(type_map);
  self->from     = from;
  self->to       = to;
  return self;
}

static void type_map_free(type_map *self) {
  if (self) {
    free(self);
  }
}

static inline void container_delete_type_map(void *self) {
  type_map_free(self);
}
static inline int container_cmp_type_map(const void *lsv, const void *rsv) {
  const type_map *l = lsv;
  const type_map *r = rsv;
  return container_cmp_ptr(l->from, r->from);
}
static inline uint64_t container_hash_type_map(const void *lsv) {
  const type_map *l = lsv;
  return container_hash_ptr(l->from);
}

HASHSET_DECLARE_STATIC_INLINE(hashset_type_map, type_map,
                              container_cmp_type_map, container_new_move,
                              container_delete_type_map,
                              container_hash_type_map);

typedef struct type_ctx_struct {
  hashset_type_map *map;
  int               type_root;
} type_ctx;

static void strbuf_append_type(type_ctx *ctx, strbuf *buffer,
                               const type_base *generic);

static void strbuf_append_type_primitive(type_ctx *ctx, strbuf *buffer,
                                         const type_primitive *type) {
  UNUSED(ctx);
  switch (type->type) {
    case TYPE_PRIMITIVE_BOOL:
      strbuf_append(buffer, "bool");
      return;
    case TYPE_PRIMITIVE_BYTE:
      strbuf_append(buffer, "byte");
      return;
    case TYPE_PRIMITIVE_INT:
      strbuf_append(buffer, "int");
      return;
    case TYPE_PRIMITIVE_UINT:
      strbuf_append(buffer, "uint");
      return;
    case TYPE_PRIMITIVE_LONG:
      strbuf_append(buffer, "long");
      return;
    case TYPE_PRIMITIVE_ULONG:
      strbuf_append(buffer, "ulong");
      return;
    case TYPE_PRIMITIVE_CHAR:
      strbuf_append(buffer, "char");
      return;
    case TYPE_PRIMITIVE_STRING:
      strbuf_append(buffer, "string");
      return;
    case TYPE_PRIMITIVE_VOID:
      strbuf_append(buffer, "void");
      return;
    case TYPE_PRIMITIVE_ANY:
      strbuf_append(buffer, "any");
      return;
  }
  warn("unexpected primitive type %d %p", type->type, type);
}

static void strbuf_append_type_array(type_ctx *ctx, strbuf *buffer,
                                     const type_array *type) {
  ctx->type_root = 0;

  strbuf_append(buffer, "array[");
  strbuf_append_type(ctx, buffer, type->element_ref);
  strbuf_append(buffer, "]");
}

static void strbuf_append_type_callable(type_ctx *ctx, strbuf *buffer,
                                        const type_callable *type) {
  ctx->type_root = 0;

  strbuf_append(buffer, "(");
  list_type_ref_it it = list_type_ref_begin(type->params);
  if (!END(it)) {
    strbuf_append_type(ctx, buffer, GET(it));
    for (NEXT(it); !END(it); NEXT(it)) {
      strbuf_append(buffer, ", ");
      strbuf_append_type(ctx, buffer, GET(it));
    }
  }
  strbuf_append(buffer, ") -> ");
  strbuf_append_type(ctx, buffer, type->ret_ref);
}

static void strbuf_append_type_class(type_ctx *ctx, strbuf *buffer,
                                     const type_class_t *type) {
  int type_root = ctx->type_root;

  ctx->type_root = 0;

  if (type_root) {
    strbuf_append(buffer, "template ");
  }
  strbuf_append(buffer, "class ");
  strbuf_append(buffer, type->id ? type->id : "(nil)");

  list_type_ref_it it = list_type_ref_begin(type->typenames);
  if (!END(it)) {
    strbuf_append(buffer, "<");
    strbuf_append_type(ctx, buffer, GET(it));
    for (NEXT(it); !END(it); NEXT(it)) {
      strbuf_append(buffer, ", ");
      strbuf_append_type(ctx, buffer, GET(it));
    }
    strbuf_append(buffer, ">");
  }

  if (type_root) {
    list_type_ref_it it = list_type_ref_begin(type->parents);
    if (!END(it)) {
      strbuf_append(buffer, " : ");
      strbuf_append_type(ctx, buffer, GET(it));
      for (NEXT(it); !END(it); NEXT(it)) {
        strbuf_append(buffer, ", ");
        strbuf_append_type(ctx, buffer, GET(it));
      }
    }
  }
}

static void strbuf_append_type_typename(type_ctx *ctx, strbuf *buffer,
                                        const type_typename *type) {
  UNUSED(ctx);
  strbuf_append(buffer, "typename ");
  strbuf_append(buffer, type->id);

  char buf[64];
  strbuf_append_f(buffer, buf, "(%p)", type->source_ref->type_entry_ref);
}

static void strbuf_append_type_mono(type_ctx *ctx, strbuf *buffer,
                                    const type_mono *type) {
  ctx->type_root = 0;

  if (type->type_ref) {
    switch (type->type_ref->kind) {
      case TYPE_PRIMITIVE:
      case TYPE_ARRAY:
      case TYPE_CALLABLE:
      case TYPE_TYPENAME:
      case TYPE_MONO:
        strbuf_append_type(ctx, buffer, type->type_ref);
        break;
      case TYPE_CLASS_T: {
        // add mapping for replacement
        const type_class_t *tmpl = (type_class_t *)type->type_ref;
        for (list_type_ref_it it   = list_type_ref_begin(type->types),
                              t_it = list_type_ref_begin(tmpl->typenames);
             !END(it) && !END(t_it); NEXT(it), NEXT(t_it)) {
          hashset_type_map_insert(ctx->map, type_map_new(GET(t_it), GET(it)));
        }

        strbuf_append_type_class(ctx, buffer, tmpl);

        // erase
        for (list_type_ref_it it = list_type_ref_begin(tmpl->typenames);
             !END(it); NEXT(it)) {
          hashset_type_map_it found =
              hashset_type_map_find(ctx->map, &(type_map){.from = GET(it)});

          if (!END(found)) {
            hashset_type_map_erase(ctx->map, found);
          }
        }
        break;
      }
    }
  }
}

static void strbuf_append_type(type_ctx *ctx, strbuf *buffer,
                               const type_base *generic) {
  if (!generic) {
    strbuf_append(buffer, "(nil)");
    return;
  }

  hashset_type_map_it found =
      hashset_type_map_find(ctx->map, &(type_map){.from = generic});

  if (!END(found)) {
    generic = GET(found)->to;
  }

  switch (generic->kind) {
    case TYPE_PRIMITIVE:
      strbuf_append_type_primitive(ctx, buffer,
                                   (const type_primitive *)generic);
      break;
    case TYPE_ARRAY:
      strbuf_append_type_array(ctx, buffer, (const type_array *)generic);
      break;
    case TYPE_CALLABLE:
      strbuf_append_type_callable(ctx, buffer, (const type_callable *)generic);
      break;
    case TYPE_CLASS_T:
      strbuf_append_type_class(ctx, buffer, (const type_class_t *)generic);
      break;
    case TYPE_TYPENAME:
      strbuf_append_type_typename(ctx, buffer, (const type_typename *)generic);
      break;
    case TYPE_MONO:
      strbuf_append_type_mono(ctx, buffer, (const type_mono *)generic);
      break;
    default:
      warn("unexpected type kind %d %p", generic->kind, generic);
      break;
  }
}

const char *type_enum_str(type_enum kind) {
  switch (kind) {
    case TYPE_PRIMITIVE:
      return "PRIMITIVE";
    case TYPE_ARRAY:
      return "ARRAY";
    case TYPE_CALLABLE:
      return "CALLABLE";
    case TYPE_CLASS_T:
      return "CLASS_T";
    case TYPE_TYPENAME:
      return "TYPENAME";
    case TYPE_MONO:
      return "MONO";
  }
  warn("unknown type kind %d", kind);
  return "UNKNOWN";
}

char *type_str(const type_base *generic) {
  if (generic) {
    strbuf *buffer = strbuf_new(16, 0);

    type_ctx ctx = {
        .map       = hashset_type_map_new(),
        .type_root = 1,
    };

    strbuf_append_type(&ctx, buffer, generic);

    hashset_type_map_free(ctx.map);

    return strbuf_detach(buffer);
  }
  return NULL;
}

const char *type_mono_enum_str(type_mono_enum spec) {
  switch (spec) {
    case TYPE_MONO_UNSET:
      return "UNSET";
    case TYPE_MONO_FULL:
      return "FULL";
    case TYPE_MONO_NOT_FULL:
      return "NOT_FULL";
    default:
      warn("unknown type_spec %d", spec);
      return "UNKNOWN";
  }
}
