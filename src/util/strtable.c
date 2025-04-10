#include "strtable.h"

#include "util/macro.h"
#include "util/math.h"
#include "util/strbuf.h"
#include <string.h>

static inline void strbuf_append_cell(strbuf *buffer, size_t max_len,
                                      const char *field, const char *space) {
  strbuf_append(buffer, field);
  for (size_t i = 0; i < max_len - strlen(field); ++i) {
    strbuf_append(buffer, space);
  }
}

static void strbuf_append_sep(strbuf *buffer, size_t columns, size_t *widths,
                              const char *cross_left, const char *cross_between,
                              const char *cross_right) {
  strbuf_append(buffer, cross_left);
  if (columns) {
    strbuf_append_cell(buffer, widths[0], "", STRTABLE_HORIZ);

    for (size_t i = 1; i != columns; ++i) {
      strbuf_append(buffer, cross_between);
      strbuf_append_cell(buffer, widths[i], "", STRTABLE_HORIZ);
    }
  }
  strbuf_append(buffer, cross_right);
}

static void strbuf_append_sep_top(strbuf *buffer, size_t columns,
                                  size_t *widths) {
  strbuf_append_sep(buffer, columns, widths, STRTABLE_TOPLEFT,
                    STRTABLE_CROSS_TOP, STRTABLE_TOPRIGHT "\n");
}

static void strbuf_append_sep_top_title(strbuf *buffer, size_t columns,
                                        size_t *widths) {
  strbuf_append_sep(buffer, columns, widths, STRTABLE_TOPLEFT, STRTABLE_HORIZ,
                    STRTABLE_TOPRIGHT "\n");
}

static void strbuf_append_sep_between(strbuf *buffer, size_t columns,
                                      size_t *widths) {
  strbuf_append_sep(buffer, columns, widths, STRTABLE_CROSS_LEFT,
                    STRTABLE_CROSS_MID, STRTABLE_CROSS_RIGHT "\n");
}

static void strbuf_append_sep_between_title(strbuf *buffer, size_t columns,
                                            size_t *widths) {
  strbuf_append_sep(buffer, columns, widths, STRTABLE_CROSS_LEFT,
                    STRTABLE_CROSS_TOP, STRTABLE_CROSS_RIGHT "\n");
}

static void strbuf_append_sep_bottom(strbuf *buffer, size_t columns,
                                     size_t *widths) {
  strbuf_append_sep(buffer, columns, widths, STRTABLE_BOTLEFT,
                    STRTABLE_CROSS_BOT, STRTABLE_BOTRIGHT "\n");
}

static void strbuf_append_title(strbuf *buffer, size_t columns, size_t *widths,
                                const char *title) {
  size_t title_size = strlen(title);

  size_t max_title_size = columns == 0 ? columns : columns - 1;
  for (size_t i = 0; i < columns; ++i) {
    max_title_size += widths[i];
  }

  char buf[sizeof(size_t)];
  buf[1] = '\0';

  strbuf_append(buffer, STRTABLE_VERT);

  for (size_t i = 0; i < max_title_size; ++i) {
    if (i < title_size) {
      buf[0] = title[i];
    } else {
      buf[0] = STRTABLE_SPACE[0];
    }
    strbuf_append(buffer, buf);
  }

  strbuf_append(buffer, STRTABLE_VERT "\n");
}

static void strbuf_append_header(strbuf *buffer, size_t columns, size_t *widths,
                                 const list *header) {
  strbuf_append(buffer, STRTABLE_VERT);
  if (columns) {
    list_it it = list_begin((list *)header);

    strbuf_append_cell(buffer, widths[0], list_it_get(&it), STRTABLE_SPACE);
    list_it_next(&it);

    for (size_t i = 1; !list_it_end(&it); list_it_next(&it), ++i) {
      strbuf_append(buffer, STRTABLE_VERT);
      strbuf_append_cell(buffer, widths[i], list_it_get(&it), STRTABLE_SPACE);
    }
  }
  strbuf_append(buffer, STRTABLE_VERT "\n");
}

static void strbuf_append_row(strbuf *buffer, size_t columns, size_t *widths,
                              list *row) {
  strbuf_append(buffer, STRTABLE_VERT);
  list_it it = list_begin(row);
  size_t  i  = 0;

  if (!list_it_end(&it)) {
    strbuf_append_cell(buffer, widths[0], list_it_get(&it), STRTABLE_SPACE);
    list_it_next(&it);
    ++i;
  }

  for (; !list_it_end(&it) && i != columns; list_it_next(&it), ++i) {
    strbuf_append(buffer, STRTABLE_VERT);
    strbuf_append_cell(buffer, widths[i], list_it_get(&it), STRTABLE_SPACE);
  }
  for (; i < columns; ++i) {
    strbuf_append(buffer, STRTABLE_VERT);
    strbuf_append_cell(buffer, widths[i], "", STRTABLE_SPACE);
  }

  strbuf_append(buffer, STRTABLE_VERT "\n");
}

// trims title to table width
static void strtable_calc_bounds(size_t *columns_out, size_t **widths_out,
                                 const char *title, const list *header,
                                 const list *rows) {
  UNUSED(title);

  size_t columns = 0;
  for (list_it it = list_begin((list *)header); !list_it_end(&it);
       list_it_next(&it)) {
    ++columns;
  }

  size_t *widths = MALLOCN(size_t, columns);
  for (size_t i = 0; i != columns; ++i) {
    widths[i] = 0;
  }

  size_t i = 0;
  for (list_it it = list_begin((list *)header); !list_it_end(&it);
       list_it_next(&it), ++i) {
    widths[i] = strlen(list_it_get(&it));
  }

  for (list_it rows_it = list_begin((list *)rows); !list_it_end(&rows_it);
       list_it_next(&rows_it)) {
    size_t i = 0;
    for (list_it it = list_begin(list_it_get(&rows_it));
         !list_it_end(&it) && i != columns; list_it_next(&it), ++i) {
      widths[i] = max_size_t(widths[i], strlen(list_it_get(&it)));
    }
  }

  *columns_out = columns;
  *widths_out  = widths;
}

char *strtable_build(const char *title, const list *header, const list *rows) {
  strbuf *buffer = strbuf_new(0, 0);

  size_t  columns;
  size_t *widths;
  strtable_calc_bounds(&columns, &widths, title, header, rows);

  if (title) {
    strbuf_append_sep_top_title(buffer, columns, widths);
    strbuf_append_title(buffer, columns, widths, title);
    strbuf_append_sep_between_title(buffer, columns, widths);
  } else {
    strbuf_append_sep_top(buffer, columns, widths);
  }

  strbuf_append_header(buffer, columns, widths, header);

  for (list_it rows_it = list_begin((list *)rows); !list_it_end(&rows_it);
       list_it_next(&rows_it)) {
    strbuf_append_sep_between(buffer, columns, widths);
    strbuf_append_row(buffer, columns, widths, list_it_get(&rows_it));
  }
  strbuf_append_sep_bottom(buffer, columns, widths);

  free(widths);
  return strbuf_detach(buffer);
}
