#include "type.h"

#include "util/log.h"
#include "util/macro.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void hir_type_base_deinit(hir_type_base *self) { span_free(self->span); }

static void hir_type_base_free(hir_type_base *self) {
  if (self) {
    hir_type_base_deinit(self);
    free(self);
  }
}

static void hir_type_base_init(hir_type_base *self, hir_type_enum type,
                               span *span) {
  self->type = type;
  self->span = span;
}

static void hir_type_custom_free(hir_type_custom *self) {
  if (self) {
    if (self->name) {
      free(self->name);
    }
    list_hir_type_free(self->templates);
    hir_type_base_deinit(&self->base);
    free(self);
  }
}

static void hir_type_array_free(hir_type_array *self) {
  if (self) {
    hir_type_free(self->elem_type);
    hir_type_base_deinit(&self->base);
    free(self);
  }
}

static inline uint64_t hash_combine(uint64_t seed, uint64_t value) {
  return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

hir_type_base *hir_type_base_new(span *span, hir_type_enum type) {
  hir_type_base *self = MALLOC(hir_type_base);
  hir_type_base_init(self, type, span);
  return self;
}

void hir_type_free(hir_type_base *generic) {
  if (generic) {
    switch (generic->type) {
      case HIR_TYPE_BOOL:
      case HIR_TYPE_BYTE:
      case HIR_TYPE_INT:
      case HIR_TYPE_UINT:
      case HIR_TYPE_LONG:
      case HIR_TYPE_ULONG:
      case HIR_TYPE_CHAR:
      case HIR_TYPE_STRING:
      case HIR_TYPE_VOID:
      case HIR_TYPE_ANY:
        return hir_type_base_free(generic);
      case HIR_TYPE_CUSTOM:
        return hir_type_custom_free((hir_type_custom *)generic);
      case HIR_TYPE_ARRAY:
        return hir_type_array_free((hir_type_array *)generic);
    }
    error("unknown hir type %d", generic->type);
  }
}

hir_type_base *hir_type_copy(const hir_type_base *self) {
  if (!self) {
    return NULL;
  }
  switch (self->type) {
    case HIR_TYPE_BOOL:
    case HIR_TYPE_BYTE:
    case HIR_TYPE_INT:
    case HIR_TYPE_UINT:
    case HIR_TYPE_LONG:
    case HIR_TYPE_ULONG:
    case HIR_TYPE_CHAR:
    case HIR_TYPE_STRING:
    case HIR_TYPE_VOID:
    case HIR_TYPE_ANY:
      return hir_type_base_new(span_copy(self->span), self->type);
    case HIR_TYPE_CUSTOM: {
      const hir_type_custom *custom = (typeof(custom))self;
      return (hir_type_base *)hir_type_custom_new(
          span_copy(self->span), strdup(custom->name),
          hir_type_copy_list(custom->templates));
    }
    case HIR_TYPE_ARRAY: {
      const hir_type_array *array = (typeof(array))self;
      return (hir_type_base *)hir_type_array_new(
          span_copy(self->span), hir_type_copy(array->elem_type));
    }
  }

  error("unknown hir type %d", self->type);
  return NULL;
}

int hir_type_cmp(const hir_type_base *lsv, const hir_type_base *rsv) {
  if (lsv == rsv) {
    return 0;
  }

  if (lsv->type > rsv->type) {
    return 1;
  } else if (lsv->type < rsv->type) {
    return -1;
  }

  switch (lsv->type) {
    case HIR_TYPE_BOOL:
    case HIR_TYPE_BYTE:
    case HIR_TYPE_INT:
    case HIR_TYPE_UINT:
    case HIR_TYPE_LONG:
    case HIR_TYPE_ULONG:
    case HIR_TYPE_CHAR:
    case HIR_TYPE_STRING:
    case HIR_TYPE_VOID:
    case HIR_TYPE_ANY:
      return 0;
    case HIR_TYPE_CUSTOM: {
      const hir_type_custom *l = (typeof(l))lsv;
      const hir_type_custom *r = (typeof(r))rsv;

      int cmp = strcmp(l->name, r->name);
      if (cmp) {
        return cmp;
      }
      return hir_type_cmp_list(l->templates, r->templates);
    }
    case HIR_TYPE_ARRAY: {
      const hir_type_array *l = (typeof(l))lsv;
      const hir_type_array *r = (typeof(r))rsv;
      return hir_type_cmp(l->elem_type, r->elem_type);
    }
  }
  error("unexpected hir type kind %d %p", lsv->type, lsv);
  return 0;
}

uint64_t hir_type_hash(const hir_type_base *lsv) {
  if (!lsv) {
    return 0;
  }

  uint64_t hash = 5381;

  hash = hash_combine(hash, (uint64_t)lsv->type);

  switch (lsv->type) {
    case HIR_TYPE_BOOL:
    case HIR_TYPE_BYTE:
    case HIR_TYPE_INT:
    case HIR_TYPE_UINT:
    case HIR_TYPE_LONG:
    case HIR_TYPE_ULONG:
    case HIR_TYPE_CHAR:
    case HIR_TYPE_STRING:
    case HIR_TYPE_VOID:
    case HIR_TYPE_ANY:
      return hash;
    case HIR_TYPE_CUSTOM: {
      const hir_type_custom *l = (typeof(l))lsv;
      hash = hash_combine(hash, container_hash_chars(l->name));
      return hash_combine(hash, hir_type_hash_list(l->templates));
    }
    case HIR_TYPE_ARRAY: {
      const hir_type_array *l = (typeof(l))lsv;
      return hash_combine(hash, hir_type_hash(l->elem_type));
    }
  }

  error("unexpected hir type kind %d %p", lsv->type, lsv);
  return 0;
}

list_hir_type *hir_type_copy_list(const list_hir_type *in) {
  if (!in) {
    return NULL;
  }
  list_hir_type *nlist = list_hir_type_new();
  for (list_hir_type_it it = list_hir_type_begin(in); !END(it); NEXT(it)) {
    list_hir_type_push_back(nlist, hir_type_copy(GET(it)));
  }
  return nlist;
}

int hir_type_cmp_list(const list_hir_type *lsv, const list_hir_type *rsv) {
  if (lsv == rsv) {
    return 0;
  }

  size_t lsz = lsv ? list_hir_type_size(lsv) : 0;
  size_t rsz = rsv ? list_hir_type_size(rsv) : 0;
  if (lsz > rsz) {
    return 1;
  } else if (lsz < rsz) {
    return -1;
  } else if (!lsz) {
    return 0;
  }

  for (list_hir_type_it lit = list_hir_type_begin(lsv),
                        rit = list_hir_type_begin(rsv);
       !END(lit) && !END(rit); NEXT(lit), NEXT(rit)) {
    int cmp = hir_type_cmp(GET(lit), GET(rit));
    if (cmp) {
      return cmp;
    }
  }

  return 0;
}

uint64_t hir_type_hash_list(const list_hir_type *list) {
  uint64_t hash = 5381;
  if (!list) {
    return hash;
  }
  for (list_hir_type_it it = list_hir_type_begin(list); !END(it); NEXT(it)) {
    hash = hash_combine(hash, hir_type_hash(GET(it)));
  }
  return hash;
}

hir_type_custom *hir_type_custom_new(span *span, char *name,
                                     list_hir_type *templates) {
  hir_type_custom *self = MALLOC(hir_type_custom);
  hir_type_base_init(&self->base, HIR_TYPE_CUSTOM, span);
  self->name      = name;
  self->templates = templates;
  return self;
}

hir_type_array *hir_type_array_new(span *span, hir_type_base *elem_type) {
  hir_type_array *self = MALLOC(hir_type_array);
  hir_type_base_init(&self->base, HIR_TYPE_ARRAY, span);
  self->elem_type = elem_type;
  return self;
}
