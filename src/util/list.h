#pragma once

#include "util/container.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct list_node {
  void             *data;
  struct list_node *next;
} list_node;

typedef struct list_struct {
  list_node *head;
  list_node *tail;

  container_f_cmp    *f_cmp;
  container_f_new    *f_new;
  container_f_delete *f_delete;
} list;

typedef struct list_it_struct {
  struct list_node *cur;
} list_it;

void    list_init(list *self, container_f_cmp, container_f_new,
                  container_f_delete);
list   *list_new(container_f_cmp, container_f_new, container_f_delete);
void    list_deinit(list *self);
void    list_free(list *self);
void    list_push_back(list *self, void *data);
void    list_push_front(list *self, void *data);
void   *list_pop_front(list *self);
void   *list_front(const list *self);
void   *list_back(const list *self);
int     list_empty(const list *self);
list_it list_find(const list *self, const void *data);
void    list_insert(list *self, list_it it, void *data);
size_t  list_size(const list *self);
list_it list_begin(list *self);

void *list_it_get(list_it *it);
int   list_it_next(list_it *it);
int   list_it_end(list_it *it);

#define LIST_DECLARE_STATIC_INLINE(list_type, type, cmp_func, new_func,        \
                                   del_func)                                   \
                                                                               \
  typedef struct list_type##_struct {                                          \
    list list;                                                                 \
  } list_type;                                                                 \
                                                                               \
  typedef struct list_type##_it_struct {                                       \
    list_it it;                                                                \
    type *(*get)(struct list_type##_it_struct * it);                           \
    int (*next)(struct list_type##_it_struct * it);                            \
    int (*end)(struct list_type##_it_struct * it);                             \
  } list_type##_it;                                                            \
                                                                               \
  __attribute__((__unused__)) static inline type *list_type##_it_get(          \
      list_type##_it *self) {                                                  \
    return list_it_get(&self->it);                                             \
  }                                                                            \
  __attribute__((__unused__)) static inline int list_type##_it_next(           \
      list_type##_it *self) {                                                  \
    return list_it_next(&self->it);                                            \
  }                                                                            \
  __attribute__((__unused__)) static inline int list_type##_it_end(            \
      list_type##_it *self) {                                                  \
    return list_it_end(&self->it);                                             \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline list_type *list_type##_new() {     \
    list_type *self = (list_type *)malloc(sizeof(list_type));                  \
    list_init(&self->list, cmp_func, new_func, del_func);                      \
    return self;                                                               \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void list_type##_free(             \
      list_type *self) {                                                       \
    if (self) {                                                                \
      list_deinit(&self->list);                                                \
      free(self);                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void list_type##_push_back(        \
      list_type *self, type *data) {                                           \
    list_push_back(&self->list, (void *)data);                                 \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void list_type##_push_front(       \
      list_type *self, type *data) {                                           \
    list_push_front(&self->list, (void *)data);                                \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline type *list_type##_pop_front(       \
      list_type *self) {                                                       \
    return (type *)list_pop_front(&self->list);                                \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline type *list_type##_front(           \
      const list_type *self) {                                                 \
    return (type *)list_front(&self->list);                                    \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline type *list_type##_back(            \
      const list_type *self) {                                                 \
    return (type *)list_back(&self->list);                                     \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline int list_type##_empty(             \
      const list_type *self) {                                                 \
    return list_empty(&self->list);                                            \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline list_type##_it list_type##_find(   \
      const list_type *self, const type *data) {                               \
    return (list_type##_it){                                                   \
        .it   = list_find(&self->list, (const void *)data),                    \
        .get  = list_type##_it_get,                                            \
        .next = list_type##_it_next,                                           \
        .end  = list_type##_it_end,                                            \
    };                                                                         \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline void list_type##_insert(           \
      list_type *self, list_type##_it it, type *data) {                        \
    list_insert(&self->list, it.it, (void *)data);                             \
  }                                                                            \
                                                                               \
  __attribute__((__unused__)) static inline size_t list_type##_size(           \
      const list_type *self) {                                                 \
    return list_size(&self->list);                                             \
  }                                                                            \
  __attribute__((__unused__)) static inline list_type##_it list_type##_begin(  \
      const list_type *self) {                                                 \
    return (list_type##_it){                                                   \
        .it   = list_begin((list *)&self->list),                               \
        .get  = list_type##_it_get,                                            \
        .next = list_type##_it_next,                                           \
        .end  = list_type##_it_end,                                            \
    };                                                                         \
  }
