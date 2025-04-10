#include "util/strbuf.h"
#include "util/log.h"
#include "util/macro.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strbuf_free(strbuf *buffer) {
  if (buffer) {
    free(buffer->data);
    free(buffer);
  }
}

strbuf *strbuf_new(int capacity, double growth_factor) {
  strbuf *self = MALLOC(strbuf);
  if (capacity > 0) {
    self->capacity = capacity;
  } else {
    self->capacity = STRBUF_INITIAL_CAPACITY;
  }
  self->data    = malloc(self->capacity);
  self->data[0] = '\0';
  self->cur     = self->data;
  if (growth_factor > 1) {
    self->growth_factor = growth_factor;
  } else {
    self->growth_factor = STRBUF_GROWTH_FACTOR;
  }
  return self;
}

static void strbuf_resize(strbuf *self) {
  size_t new_capacity = self->capacity * self->growth_factor;
  size_t size         = strbuf_size(self);

  if (new_capacity < self->capacity) {
    error("strbuf new_capacity is less than previous");
  }

  self->data     = realloc(self->data, new_capacity);
  self->cur      = self->data + (size / sizeof(char));
  self->capacity = new_capacity;
}

void strbuf_append(strbuf *self, const char *data) {
  size_t data_size = strlen(data) * sizeof(char);
  size_t size      = strbuf_size(self);

  while (size + data_size >= self->capacity) {
    strbuf_resize(self);
  }

  memcpy(self->cur, data, data_size + 1); // copy with null terminator
  self->cur += (data_size / sizeof(char));
}

void strbuf_append_f(strbuf *self, char *buf, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);
  strbuf_append(self, buf);
}

char *strbuf_data(strbuf *self) { return self->data; }

char *strbuf_detach(strbuf *self) {
  char *data = NULL;
  if (self) {
    data = strbuf_data(self);
    free(self);
  }
  return data;
}

size_t strbuf_size(strbuf *self) {
  return (((intptr_t)self->cur) - ((intptr_t)self->data));
}

void strbuf_reset(strbuf *self) {
  self->cur  = self->data;
  *self->cur = '\0';
}
