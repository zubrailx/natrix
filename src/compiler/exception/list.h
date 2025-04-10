#pragma once

#include "compiler/exception/exception.h"
#include "util/container_util.h"
#include "util/list.h"

static inline void *container_new_exception(void *_data) { return _data; }
static inline void  container_delete_exception(void *_data) {
  exception_free(_data);
}
LIST_DECLARE_STATIC_INLINE(list_exception, exception, container_cmp_false,
                           container_new_exception, container_delete_exception);

size_t list_exception_count_by_type(const list_exception *self,
                                    exception_type        type);
size_t list_exception_count_by_level(const list_exception *self,
                                     exception_level       level);
void   list_exception_extend(list_exception *self, list_exception *list);
