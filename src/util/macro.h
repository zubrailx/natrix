#pragma once

#define UNUSED(x) (void)(x)
#define MALLOC(x) ((typeof(x) *)malloc(sizeof(typeof(x))))
#define MALLOCN(x, n) ((typeof(x) *)malloc(sizeof(typeof(x)) * n))
#define STRMAXLEN(x) (sizeof(x) - 1)

#define CASSERT(predicate, file) _impl_CASSERT_LINE(predicate, __LINE__, file)

#define _impl_PASTE(a, b) a##b
#define _impl_CASSERT_LINE(predicate, line, file)                              \
  typedef char _impl_PASTE(assertion_failed_##file##_,                         \
                           line)[2 * !!(predicate) - 1];

#define GET(x) (x.get(&x))
#define NEXT(x) (x.next(&x))
#define END(x) (x.end(&x))
