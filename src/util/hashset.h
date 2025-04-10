#pragma once

#include "util/container.h"
#include <stddef.h>
#include <stdlib.h>

#define HASHTABLE_LOAD_FACTOR 0.75
#define HASHTABLE_GROWTH_FACTOR 2
#define HASHTABLE_INITIAL_CAPACITY 16

typedef struct hashset_list_node_struct {
  void                            *data;
  struct hashset_list_node_struct *next;
} hashset_list_node;

typedef struct hashset_bucket_struct {
  hashset_list_node *head;
} hashset_bucket;

typedef struct hashset_struct {
  hashset_bucket *buckets;

  double load_factor;
  double growth_factor;

  size_t capacity;
  size_t size;

  container_f_cmp    *f_cmp;
  container_f_new    *f_new;
  container_f_delete *f_delete;
  container_f_hash   *f_hash;
} hashset;

typedef struct hashset_it_struct {
  hashset           *hashset;
  size_t             bucket_i;
  hashset_list_node *node;
  hashset_list_node *prev; // previous in the bucket
} hashset_it;

void       hashset_init(hashset *self, container_f_cmp, container_f_new,
                        container_f_delete, container_f_hash);
hashset   *hashset_new(container_f_cmp, container_f_new, container_f_delete,
                       container_f_hash);
void       hashset_deinit(hashset *self);
void       hashset_free(hashset *self);
hashset_it hashset_find(hashset *self, void *data);
hashset_it hashset_insert(hashset *self, void *data);
hashset_it hashset_begin(hashset *self);
void       hashset_replace(hashset *self, hashset_it it, void *data);
// NOTE: consumes hashset (it invalidates)
void hashset_erase(hashset *self, hashset_it it);

void *hashset_it_get(hashset_it *it);
int   hashset_it_end(hashset_it *it);
int   hashset_it_next(hashset_it *it);

#define HASHSET_DECLARE_STATIC_INLINE(hashset_type, type, cmp_func, new_func,  \
                                      del_func, hash_func)                     \
                                                                               \
  typedef struct hashset_type##_struct {                                       \
    hashset hashset;                                                           \
  } hashset_type;                                                              \
                                                                               \
  typedef struct hashset_type##_it_struct {                                    \
    hashset_it it;                                                             \
    type *(*get)(struct hashset_type##_it_struct * it);                        \
    int (*next)(struct hashset_type##_it_struct * it);                         \
    int (*end)(struct hashset_type##_it_struct * it);                          \
  } hashset_type##_it;                                                         \
                                                                               \
  __attribute__((__unused__)) static inline type *hashset_type##_it_get(       \
      hashset_type##_it *self) {                                               \
    return hashset_it_get(&self->it);                                          \
  }                                                                            \
  __attribute__((__unused__)) static inline int hashset_type##_it_next(        \
      hashset_type##_it *self) {                                               \
    return hashset_it_next(&self->it);                                         \
  }                                                                            \
  __attribute__((__unused__)) static inline int hashset_type##_it_end(         \
      hashset_type##_it *self) {                                               \
    return hashset_it_end(&self->it);                                          \
  }                                                                            \
                                                                               \
  __attribute__((                                                              \
      __unused__)) static inline hashset_type *hashset_type##_new() {          \
    hashset_type *self = (hashset_type *)malloc(sizeof(hashset_type));         \
    hashset_init(&self->hashset, cmp_func, new_func, del_func, hash_func);     \
    return self;                                                               \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void hashset_type##_free(          \
      hashset_type *self) {                                                    \
    if (self) {                                                                \
      hashset_deinit(&self->hashset);                                          \
      free(self);                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline hashset_type##_it                  \
      hashset_type##_find(hashset_type *self, type *data) {                    \
    return (hashset_type##_it){                                                \
        .it   = hashset_find(&self->hashset, (void *)data),                    \
        .get  = hashset_type##_it_get,                                         \
        .next = hashset_type##_it_next,                                        \
        .end  = hashset_type##_it_end,                                         \
    };                                                                         \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline hashset_type##_it                  \
      hashset_type##_insert(hashset_type *self, type *data) {                  \
    return (hashset_type##_it){                                                \
        .it   = hashset_insert(&self->hashset, (void *)data),                  \
        .get  = hashset_type##_it_get,                                         \
        .next = hashset_type##_it_next,                                        \
        .end  = hashset_type##_it_end,                                         \
    };                                                                         \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline hashset_type##_it                  \
      hashset_type##_begin(const hashset_type *self) {                         \
    return (hashset_type##_it){                                                \
        .it   = hashset_begin((hashset *)&self->hashset),                      \
        .get  = hashset_type##_it_get,                                         \
        .next = hashset_type##_it_next,                                        \
        .end  = hashset_type##_it_end,                                         \
    };                                                                         \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void hashset_type##_replace(       \
      hashset_type *self, hashset_type##_it it, type *data) {                  \
    hashset_replace(&self->hashset, it.it, (void *)data);                      \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void hashset_type##_erase(         \
      hashset_type *self, hashset_type##_it it) {                              \
    hashset_erase(&self->hashset, it.it);                                      \
  }
