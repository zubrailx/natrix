#include "dot.h"
#include "util/macro.h"
#include <stdlib.h>

void dot_string_free(dot_string *str) {
  if (str) {
    if (str->chars) {
      free(str->chars);
    }
    free(str);
  }
}

dot_string *dot_string_new(char *chars, size_t len) {
  dot_string *str = MALLOC(dot_string);
  str->chars      = chars;
  str->len        = len;
  return str;
}
