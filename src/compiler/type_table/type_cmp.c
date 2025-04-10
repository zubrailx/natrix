#include "type.h"

#include "compiler/type_table/type.h"
#include "util/log.h"
#include "util/macro.h"
#include <string.h>

static inline uint64_t hash_combine(uint64_t seed, uint64_t value) {
  return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

// check if null because call
int type_cmp(const type_base *lsv, const type_base *rsv) {
  if (lsv == rsv) {
    return 0;
  }
  if (lsv->kind > rsv->kind) {
    return 1;
  }
  if (lsv->kind < rsv->kind) {
    return -1;
  }
  switch (lsv->kind) {
    case TYPE_PRIMITIVE: {
      const type_primitive *l = (const type_primitive *)lsv;
      const type_primitive *r = (const type_primitive *)rsv;
      if (l->type == r->type) {
        return 0;
      } else if (l->type > r->type) {
        return 1;
      } else {
        return -1;
      }
    }
    case TYPE_ARRAY: {
      const type_array *l = (const type_array *)lsv;
      const type_array *r = (const type_array *)rsv;
      return type_cmp(l->element_ref, r->element_ref);
    } break;
    case TYPE_CALLABLE: {
      const type_callable *l = (const type_callable *)lsv;
      const type_callable *r = (const type_callable *)rsv;

      size_t lsz = list_type_ref_size(l->params);
      size_t rsz = list_type_ref_size(r->params);
      if (lsz > rsz) {
        return 1;
      } else if (lsz < rsz) {
        return -1;
      }
      for (list_type_ref_it lit = list_type_ref_begin(l->params),
                            rit = list_type_ref_begin(r->params);
           !END(lit); NEXT(lit), NEXT(rit)) {
        int cmp = type_cmp(GET(lit), GET(rit));
        if (cmp) {
          return cmp;
        }
      }

      return type_cmp(l->ret_ref, r->ret_ref);
    }
    case TYPE_CLASS_T: {
      const type_class_t *l = (const type_class_t *)lsv;
      const type_class_t *r = (const type_class_t *)rsv;

      int cmp = strcmp(l->id, r->id);
      if (cmp) {
        return cmp;
      }

      // parents
      size_t lsz = list_type_ref_size(l->parents);
      size_t rsz = list_type_ref_size(r->parents);
      if (lsz > rsz) {
        return 1;
      } else if (lsz < rsz) {
        return -1;
      }
      for (list_type_ref_it lit = list_type_ref_begin(l->parents),
                            rit = list_type_ref_begin(r->parents);
           !END(lit); NEXT(lit), NEXT(rit)) {
        int cmp = type_cmp(GET(lit), GET(rit));
        if (cmp) {
          return cmp;
        }
      }

      // typenames
      lsz = list_type_ref_size(l->typenames);
      rsz = list_type_ref_size(r->typenames);
      if (lsz > rsz) {
        return 1;
      } else if (lsz < rsz) {
        return -1;
      }
      for (list_type_ref_it lit = list_type_ref_begin(l->typenames),
                            rit = list_type_ref_begin(r->typenames);
           !END(lit); NEXT(lit), NEXT(rit)) {
        int cmp = type_cmp(GET(lit), GET(rit));
        if (cmp) {
          return cmp;
        }
      }
      return 0;
    }
    case TYPE_TYPENAME: {
      const type_typename *l = (const type_typename *)lsv;
      const type_typename *r = (const type_typename *)rsv;

      int cmp = strcmp(l->id, r->id);
      if (cmp) {
        return cmp;
      }

      return container_cmp_ptr(l->source_ref, r->source_ref);
    }
    case TYPE_MONO: {
      const type_mono *l = (const type_mono *)lsv;
      const type_mono *r = (const type_mono *)rsv;

      int cmp = type_cmp(l->type_ref, r->type_ref);
      if (cmp) {
        return cmp;
      }

      size_t lsz = list_type_ref_size(l->types);
      size_t rsz = list_type_ref_size(r->types);
      if (lsz > rsz) {
        return 1;
      } else if (lsz < rsz) {
        return -1;
      }
      for (list_type_ref_it lit = list_type_ref_begin(l->types),
                            rit = list_type_ref_begin(r->types);
           !END(lit); NEXT(lit), NEXT(rit)) {
        int cmp = type_cmp(GET(lit), GET(rit));
        if (cmp) {
          return cmp;
        }
      }
      return 0;
    }
    default:
      error("unexpected type kind %d", lsv->kind);
      return -1;
  }
}

uint64_t type_hash(const type_base *lsv) {
  if (lsv == NULL) {
    return 0;
  }

  uint64_t hash = 5381;

  hash = hash_combine(hash, (uint64_t)lsv->kind);

  switch (lsv->kind) {
    case TYPE_PRIMITIVE: {
      const type_primitive *l = (const type_primitive *)lsv;
      hash                    = hash_combine(hash, (uint64_t)l->type);
    } break;
    case TYPE_ARRAY: {
      const type_array *l = (const type_array *)lsv;
      hash                = hash_combine(hash, type_hash(l->element_ref));
    } break;
    case TYPE_CALLABLE: {
      const type_callable *l = (const type_callable *)lsv;

      for (list_type_ref_it it = list_type_ref_begin(l->params); !END(it);
           NEXT(it)) {
        hash = hash_combine(hash, type_hash(GET(it)));
      }

      hash = hash_combine(hash, type_hash(l->ret_ref));
    } break;
    case TYPE_CLASS_T: {
      const type_class_t *l = (const type_class_t *)lsv;

      hash = hash_combine(hash, container_hash_chars(l->id));

      for (list_type_ref_it it = list_type_ref_begin(l->parents); !END(it);
           NEXT(it)) {
        hash = hash_combine(hash, type_hash(GET(it)));
      }

      for (list_type_ref_it it = list_type_ref_begin(l->typenames); !END(it);
           NEXT(it)) {
        hash = hash_combine(hash, type_hash(GET(it)));
      }
    } break;
    case TYPE_TYPENAME: {
      const type_typename *l = (const type_typename *)lsv;

      hash = hash_combine(hash, container_hash_chars(l->id));

      hash = hash_combine(hash, container_hash_ptr(l->source_ref));
    } break;
    case TYPE_MONO: {
      const type_mono *l = (const type_mono *)lsv;

      hash = hash_combine(hash, type_hash(l->type_ref));

      for (list_type_ref_it it = list_type_ref_begin(l->types); !END(it);
           NEXT(it)) {
        hash = hash_combine(hash, type_hash(GET(it)));
      }
    } break;
    default:
      error("unexpected type kind %d", lsv->kind);
      return 0;
  }

  return hash;
}
