#include "str.h"
#include "util/macro.h"
#include "util/strbuf.h"
#include <stdio.h>

char *span_str(const span *span) {
  char    buf[32];
  strbuf *buffer = strbuf_new(32, 0);

  strbuf_append(buffer, span->source_ref);
  strbuf_append(buffer, " ");
  snprintf(buf, STRMAXLEN(buf), "%zu:%zu", span->line_start, span->pos_start);
  strbuf_append(buffer, buf);
  strbuf_append(buffer, " ");
  snprintf(buf, STRMAXLEN(buf), "%zu:%zu", span->line_end, span->pos_end);
  strbuf_append(buffer, buf);

  return strbuf_detach(buffer);
}
