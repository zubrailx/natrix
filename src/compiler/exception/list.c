#include "list.h"
#include "util/macro.h"

size_t list_exception_count_by_type(const list_exception *list,
                                    exception_type        type) {
  size_t cnt = 0;
  for (list_exception_it it = list_exception_begin((list_exception *)list);
       !END(it); NEXT(it)) {
    exception *exc = GET(it);
    if (exc->type == type) {
      ++cnt;
    }
  }
  return cnt;
}

size_t list_exception_count_by_level(const list_exception *list,
                                     exception_level       level) {
  if (!list) {
    return 0;
  }

  size_t cnt = 0;
  for (list_exception_it it = list_exception_begin((list_exception *)list);
       !END(it); NEXT(it)) {
    exception *exc = GET(it);
    if (exc->level == level) {
      ++cnt;
    }
  }
  return cnt;
}

void list_exception_extend(list_exception *self, list_exception *list) {
  if (!self || !list) {
    return;
  }

  while (!list_exception_empty(list)) {
    list_exception_push_back(self, list_exception_pop_front(list));
  }
  list_exception_free(list);
}
