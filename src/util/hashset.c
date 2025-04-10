#include "util/hashset.h"

#include "util/macro.h"
#include <stdlib.h>

static void hashset_bucket_free_in(hashset *self, hashset_bucket *bucket) {
  hashset_list_node *next;
  for (hashset_list_node *node = bucket->head; node; node = next) {
    next = node->next;
    self->f_delete(node->data);
    free(node);
  }
}

static hashset_it hashset_it_make(hashset *hashset, size_t bucket_i,
                                  hashset_list_node *node,
                                  hashset_list_node *prev) {
  hashset_it it = {
      .hashset  = hashset,
      .bucket_i = bucket_i,
      .node     = node,
      .prev     = prev,
  };
  return it;
}

static void hashset_grow(hashset *self) {
  size_t new_capacity = self->capacity * self->growth_factor;

  hashset_bucket *new_buckets = MALLOCN(hashset_bucket, new_capacity);
  for (size_t i = 0; i < new_capacity; ++i) {
    new_buckets[i].head = NULL;
  }

  for (size_t i = 0; i < self->capacity; ++i) {
    hashset_bucket    *bucket = self->buckets + i;
    hashset_list_node *next;
    for (hashset_list_node *node = bucket->head; node; node = next) {
      next                       = node->next;
      size_t          new_i      = self->f_hash(node->data) % new_capacity;
      hashset_bucket *new_bucket = new_buckets + new_i;
      node->next                 = new_bucket->head;
      new_bucket->head           = node;
    }
  }

  free(self->buckets);
  self->buckets  = new_buckets;
  self->capacity = new_capacity;
}

void hashset_init(hashset *self, container_f_cmp f_cmp, container_f_new f_new,
                  container_f_delete f_delete, container_f_hash f_hash) {
  self->load_factor   = HASHTABLE_LOAD_FACTOR;
  self->growth_factor = HASHTABLE_GROWTH_FACTOR;
  self->capacity      = HASHTABLE_INITIAL_CAPACITY;
  self->size          = 0;

  self->f_cmp    = f_cmp;
  self->f_new    = f_new;
  self->f_delete = f_delete;
  self->f_hash   = f_hash;

  self->buckets = MALLOCN(hashset_bucket, self->capacity);
  for (size_t i = 0; i < self->capacity; ++i) {
    self->buckets[i].head = NULL;
  }
}

hashset *hashset_new(container_f_cmp f_cmp, container_f_new f_new,
                     container_f_delete f_delete, container_f_hash f_hash) {
  hashset *self = MALLOC(hashset);
  hashset_init(self, f_cmp, f_new, f_delete, f_hash);
  return self;
}

void hashset_deinit(hashset *self) {
  for (size_t i = 0; i < self->capacity; ++i) {
    hashset_bucket_free_in(self, self->buckets + i);
  }
  free(self->buckets);
}

void hashset_free(hashset *self) {
  if (self) {
    hashset_deinit(self);
    free(self);
  }
}

int hashset_it_end(hashset_it *it) { return !it->node; }

void *hashset_it_get(hashset_it *it) { return it->node->data; }

int hashset_it_next(hashset_it *it) {
  if (it->node) {
    it->prev = it->node;
    it->node = it->node->next;
    if (!it->node) {
      it->prev = NULL;
      while (++it->bucket_i < it->hashset->capacity &&
             !it->hashset->buckets[it->bucket_i].head)
        ;
      if (it->bucket_i == it->hashset->capacity) {
        return 0;
      }
      it->node = it->hashset->buckets[it->bucket_i].head;
    }
    return 1;
  }
  return 0;
}

hashset_it hashset_find(hashset *self, void *data) {
  size_t          index  = self->f_hash(data) % self->capacity;
  hashset_bucket *bucket = self->buckets + index;

  hashset_list_node *prev = NULL;
  for (hashset_list_node *node = bucket->head; node; node = node->next) {
    if (!self->f_cmp(node->data, data)) {
      return hashset_it_make(self, index, node, prev);
    }
    prev = node;
  }
  return hashset_it_make(self, 0, NULL, NULL);
}

hashset_it hashset_insert(hashset *self, void *data) {
  if ((float)self->size / self->capacity > self->load_factor) {
    hashset_grow(self);
  }

  size_t          index  = self->f_hash(data) % self->capacity;
  hashset_bucket *bucket = self->buckets + index;

  hashset_list_node *prev = NULL;
  for (hashset_list_node *node = bucket->head; node; node = node->next) {
    if (!self->f_cmp(node->data, data)) {
      self->f_delete(node->data);
      node->data = self->f_new(data);
      return hashset_it_make(self, index, node, prev);
    } else {
      prev = node;
    }
  }

  hashset_list_node *node = MALLOC(hashset_list_node);
  node->data              = self->f_new(data);
  node->next              = bucket->head;
  bucket->head            = node;
  self->size += 1;
  return hashset_it_make(self, index, node, NULL);
}

void hashset_replace(hashset *self, hashset_it it, void *data) {
  self->f_delete(it.node->data);
  it.node->data = self->f_new(data);
}

hashset_it hashset_begin(hashset *self) {
  for (size_t i = 0; i < self->capacity; ++i) {
    hashset_bucket *bucket = self->buckets + i;
    if (bucket->head) {
      return hashset_it_make(self, i, bucket->head, NULL);
    }
  }
  return hashset_it_make(self, 0, NULL, NULL);
}

void hashset_erase(hashset *self, hashset_it it) {
  self->f_delete(it.node->data);
  if (it.prev) {
    it.prev->next = it.node->next;
  } else {
    self->buckets[it.bucket_i].head = it.node->next;
  }
  free(it.node);
}
