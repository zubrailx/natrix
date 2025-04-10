#include <criterion/criterion.h>
#include <criterion/options.h>

#include <stdio.h>

#include "util/container_util.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strtable.h"

LIST_DECLARE_STATIC_INLINE(list_chars, char, container_cmp_false,
                           container_new_chars, container_delete_chars);

static inline void container_delete_list_chars(void *data) {
  list_chars_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_list_chars, list_chars, container_cmp_false,
                           container_new_move, container_delete_list_chars);

Test(strtable, no_columns) {
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  list_list_chars_push_back(body, list_chars_new());

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, no_body) {
  char             buf[256];
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  snprintf(buf, STRMAXLEN(buf), "header_%zu", (size_t)0);
  list_chars_push_back(header, buf);

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, basic_one_column) {
  char             buf[256];
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  for (size_t i = 0; i != 1; ++i) {
    snprintf(buf, STRMAXLEN(buf), "header_%zu", (size_t)i);
    list_chars_push_back(header, buf);
  }

  for (size_t j = 0; j != 5; ++j) {
    list_chars *row = list_chars_new();
    for (size_t i = 0; i != 1; ++i) {
      snprintf(buf, STRMAXLEN(buf), "body_%zu_%zu", (size_t)j, (size_t)i);
      list_chars_push_back(row, buf);
    }
    list_list_chars_push_back(body, row);
  }

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, basic_many_columns_header_more_width) {
  char             buf[256];
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  for (size_t i = 0; i != 5; ++i) {
    snprintf(buf, STRMAXLEN(buf), "table_header_%zu", (size_t)i);
    list_chars_push_back(header, buf);
  }

  for (size_t j = 0; j != 5; ++j) {
    list_chars *row = list_chars_new();
    for (size_t i = 0; i != 5; ++i) {
      snprintf(buf, STRMAXLEN(buf), "body_%zu_%zu", (size_t)j, (size_t)i);
      list_chars_push_back(row, buf);
    }
    list_list_chars_push_back(body, row);
  }

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, basic_many_columns_header_less_width) {
  char             buf[256];
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  for (size_t i = 0; i != 5; ++i) {
    snprintf(buf, STRMAXLEN(buf), "header_%zu", (size_t)i);
    list_chars_push_back(header, buf);
  }

  for (size_t j = 0; j != 5; ++j) {
    list_chars *row = list_chars_new();
    for (size_t i = 0; i != 5; ++i) {
      snprintf(buf, STRMAXLEN(buf), "table_body_%zu_%zu", (size_t)j, (size_t)i);
      list_chars_push_back(row, buf);
    }
    list_list_chars_push_back(body, row);
  }

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, different_body_columns_less) {
  char             buf[256];
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  for (size_t i = 0; i != 5; ++i) {
    snprintf(buf, STRMAXLEN(buf), "table_header_%zu", (size_t)i);
    list_chars_push_back(header, buf);
  }

  for (size_t j = 0; j != 5; ++j) {
    list_chars *row = list_chars_new();
    for (size_t i = 0; i != 4; ++i) {
      snprintf(buf, STRMAXLEN(buf), "body_%zu_%zu", (size_t)j, (size_t)i);
      list_chars_push_back(row, buf);
    }
    list_list_chars_push_back(body, row);
  }

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, different_body_columns_greater) {
  char             buf[256];
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  for (size_t i = 0; i != 5; ++i) {
    snprintf(buf, STRMAXLEN(buf), "table_header_%zu", (size_t)i);
    list_chars_push_back(header, buf);
  }

  for (size_t j = 0; j != 5; ++j) {
    list_chars *row = list_chars_new();
    for (size_t i = 0; i != 6; ++i) {
      snprintf(buf, STRMAXLEN(buf), "body_%zu_%zu", (size_t)j, (size_t)i);
      list_chars_push_back(row, buf);
    }
    list_list_chars_push_back(body, row);
  }

  char *table = strtable_build(NULL, &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}

Test(strtable, title_no_body) {
  list_chars      *header = list_chars_new();
  list_list_chars *body = list_list_chars_new();

  char *table = strtable_build("Example", &header->list, &body->list);
  debug("\n%s", table);
  free(table);

  list_list_chars_free(body);
  list_chars_free(header);
}
