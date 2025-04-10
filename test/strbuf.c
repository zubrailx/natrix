#include <criterion/criterion.h>

#include "util/strbuf.h"

Test(string_buffer, test1) {
  strbuf *buffer = strbuf_new(4, 0);

  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");

  strbuf_free(buffer);
}

Test(string_buffer, test2) {
  strbuf *buffer = strbuf_new(4, 0);

  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");
  strbuf_append(buffer, "hello, world!");

  char *data = strbuf_detach(buffer);
  free(data);
}
