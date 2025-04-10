#include <stddef.h>

#define MATH(type)                                                             \
  static inline type max_##type(type a, type b) { return a > b ? a : b; }      \
  static inline type min_##type(type a, type b) { return a > b ? b : a; }

MATH(size_t)
