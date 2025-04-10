#include "compiler/mir/mir.h"
#include "util/hashset.h"

// value_meta
typedef struct cg_value_meta_struct {
  const mir_value *value_ref;
  int64_t          offset; // of stack relative to rbp
  int              is_ptr; // is pointer of actual value
} cg_value_meta;

cg_value_meta *cg_value_meta_new(const mir_value *value_ref, int64_t offset,
                                 int is_ptr);
void           cg_value_meta_free(cg_value_meta *self);

static inline int container_cmp_cg_value_meta(const void *lsv,
                                              const void *rsv) {
  const cg_value_meta *l = lsv;
  const cg_value_meta *r = rsv;
  return container_cmp_ptr(l->value_ref, r->value_ref);
}
static inline uint64_t container_hash_cg_value_meta(const void *lsv) {
  const cg_value_meta *l = lsv;
  return container_hash_ptr(l->value_ref);
}
static inline void container_delete_cg_value_meta(void *data) {
  return cg_value_meta_free(data);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_cg_value_meta, cg_value_meta,
                              container_cmp_cg_value_meta, container_new_move,
                              container_delete_cg_value_meta,
                              container_hash_cg_value_meta);

// mir_sym (to find symbols by mir pointer)
typedef struct cg_mir_sym_struct {
  union {
    const void           *mir_ref;
    const mir_lit        *lit_ref;
    const mir_subroutine *sub_ref;
  };

  const char *sym_ref;
} cg_mir_sym;

cg_mir_sym *cg_mir_sym_new(const void *mir_ref, const char *sym_ref);
void        cg_mir_sym_free(cg_mir_sym *self);

static inline int container_cmp_cg_mir_sym(const void *lsv, const void *rsv) {
  const cg_mir_sym *l = lsv;
  const cg_mir_sym *r = rsv;
  return container_cmp_ptr(l->mir_ref, r->mir_ref);
}
static inline uint64_t container_hash_cg_mir_sym(const void *lsv) {
  const cg_mir_sym *l = lsv;
  return container_hash_ptr(l->mir_ref);
}
static inline void container_delete_cg_mir_sym(void *data) {
  return cg_mir_sym_free(data);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_cg_mir_sym, cg_mir_sym,
                              container_cmp_cg_mir_sym, container_new_move,
                              container_delete_cg_mir_sym,
                              container_hash_cg_mir_sym);

// type_sym (to find symbols by type)
typedef struct cg_type_sym_struct {
  union {
    const type_base *type_ref;
    const type_mono *mono_ref;
  };

  const char *sym_ref;
} cg_type_sym;

cg_type_sym *cg_type_sym_new(const void *type_ref, const char *sym_ref);
void         cg_type_sym_free(cg_type_sym *self);

static inline int container_cmp_cg_type_sym(const void *lsv, const void *rsv) {
  const cg_type_sym *l = lsv;
  const cg_type_sym *r = rsv;
  return container_cmp_ptr(l->type_ref, r->type_ref);
}
static inline uint64_t container_hash_cg_type_sym(const void *lsv) {
  const cg_type_sym *l = lsv;
  return container_hash_ptr(l->type_ref);
}
static inline void container_delete_cg_type_sym(void *data) {
  return cg_type_sym_free(data);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_cg_type_sym, cg_type_sym,
                              container_cmp_cg_type_sym, container_new_move,
                              container_delete_cg_type_sym,
                              container_hash_cg_type_sym);

// method_sym
typedef struct cg_method_sym_struct {
  const mir_subroutine *method_ref;
  char                 *sym;
} cg_method_sym;

cg_method_sym *cg_method_sym_new(const mir_subroutine *method_ref, char *sym);
void           cg_method_sym_free(cg_method_sym *self);

static inline void container_delete_cg_method_sym(void *data) {
  return cg_method_sym_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_method_sym, cg_method_sym,
                           container_cmp_false, container_new_move,
                           container_delete_cg_method_sym);

// class_sym
typedef struct cg_class_sym_struct {
  const mir_class *class_ref;
  char            *sym;
} cg_class_sym;

cg_class_sym *cg_class_sym_new(const mir_class *class_ref, char *sym);
void          cg_class_sym_free(cg_class_sym *self);

static inline void container_delete_cg_class_sym(void *data) {
  return cg_class_sym_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_class_sym, cg_class_sym, container_cmp_false,
                           container_new_move, container_delete_cg_class_sym);
