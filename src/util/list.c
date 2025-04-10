#include <stdlib.h>
#include <string.h>

#include "util/list.h"
#include "util/macro.h"

void list_init(list *self, container_f_cmp cmp, container_f_new new,
               container_f_delete delete) {
  self->head = NULL;
  self->tail = NULL;

  self->f_cmp    = cmp;
  self->f_new    = new;
  self->f_delete = delete;
}

list *list_new(container_f_cmp cmp, container_f_new new,
               container_f_delete delete) {
  list *self = MALLOC(list);
  list_init(self, cmp, new, delete);
  return self;
}

void list_deinit(list *self) {
  list_node *next;
  for (list_node *cur = self->head; cur; cur = next) {
    next = cur->next;
    self->f_delete(cur->data);
    free(cur);
  }
}

void list_free(list *self) {
  if (self) {
    list_deinit(self);
    free(self);
  }
}

list_it list_find(const list *self, const void *data) {
  for (list_node *cur = self->head; cur; cur = cur->next) {
    if (!self->f_cmp(cur->data, data)) {
      return (list_it){.cur = cur};
    }
  }
  return (list_it){.cur = NULL};
}

void list_push_back(list *self, void *data) {
  list_node *node = MALLOC(list_node);
  node->data      = self->f_new(data);
  node->next      = NULL;

  if (self->tail) {
    self->tail->next = node;
  } else {
    self->head = node;
  }
  self->tail = node;
}

void list_push_front(list *self, void *data) {
  list_node *node = MALLOC(list_node);
  node->data      = self->f_new(data);
  node->next      = self->head;
  self->head      = node;
  if (!self->tail) {
    self->tail = node;
  }
}

void *list_front(const list *self) {
  return self->head ? self->head->data : NULL;
}

void *list_back(const list *self) {
  return self->tail ? self->tail->data : NULL;
}

int list_empty(const list *self) { return !self->head; }

// won't free data
void *list_pop_front(list *self) {
  list_node *node = self->head;
  self->head      = node->next;
  if (!self->head) {
    self->tail = NULL;
  }
  void *data = node->data;
  free(node);
  return data;
}

void list_insert(list *self, list_it it, void *data) {
  if (it.cur) {
    self->f_delete(it.cur->data);
    it.cur->data = self->f_new(data);
  } else {
    list_push_back(self, data);
  }
}

size_t list_size(const list *self) {
  size_t size = 0;
  for (list_node *cur = self->head; cur; cur = cur->next) {
    ++size;
  }
  return size;
}

list_it list_begin(list *self) { return (list_it){.cur = self->head}; }

void *list_it_get(list_it *it) { return it->cur->data; }

int list_it_next(list_it *it) {
  if (it->cur) {
    it->cur = it->cur->next;
    return it->cur ? 1 : 0;
  }
  return 0;
}

int list_it_end(list_it *it) { return !it->cur; }
