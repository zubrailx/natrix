#include <criterion/criterion.h>

#include <stdio.h>

#include "util/container_util.h"
#include "util/list.h"

Test(list, create_delete) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  cr_expect_eq(list->f_cmp, container_cmp_chars_chars);
  cr_expect_eq(list->f_new, container_new_chars_chars);
  cr_expect_eq(list->f_delete, container_delete_chars_chars);

  list_free(list);
}

Test(list, push_back) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  chars_chars data = {.key = "hello", .value = "world"};

  list_push_back(list, &data);
  list_free(list);
}

Test(list, push_back2) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  chars_chars data = {.key = "hello", .value = "world"};
  list_push_back(list, &data);
  data.key = "hello2";
  data.value = "world2";
  list_push_back(list, &data);

  int cnt = 0;
  for (list_it it = list_begin(list); !list_it_end(&it);
       list_it_next(&it), ++cnt)
    ;

  cr_expect_eq(2, cnt);

  list_free(list);
}

Test(list, find_insert) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  chars_chars data = {.key = "hello", .value = "world"};
  list_push_back(list, &data);
  data.value = "world2";
  list_insert(list, list_find(list, &data), &data);

  int cnt = 0;
  for (list_it it = list_begin(list); !list_it_end(&it);
       list_it_next(&it), ++cnt)
    ;

  cr_expect_eq(1, cnt);

  list_free(list);
}

Test(list, push_push) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  chars_chars data = {.key = "hello", .value = "world"};
  list_push_back(list, &data);
  data.value = "world2";
  list_push_back(list, &data);

  int cnt = 0;
  for (list_it it = list_begin(list); !list_it_end(&it);
       list_it_next(&it), ++cnt)
    ;

  cr_expect_eq(2, cnt);

  list_free(list);
}

Test(list, put_many) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  char key[10];
  char value[10];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    list_push_back(list, &data);
  }

  int cnt = 0;
  for (list_it it = list_begin(list); !list_it_end(&it);
       list_it_next(&it), ++cnt)
    ;

  cr_expect_eq(1000, cnt);

  list_free(list);
}

Test(list, put_many_check) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  char key[16];
  char value[16];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    list_push_back(list, &data);
  }

  int cnt = 0;
  for (list_it it = list_begin(list); !list_it_end(&it);
       list_it_next(&it), ++cnt) {
    chars_chars *data = list_it_get(&it);
    sprintf(key, "%d", cnt);
    sprintf(value, "%d", cnt + 1000);

    cr_expect_str_eq(key, data->key);
    cr_expect_str_eq(value, data->value);
  }

  cr_expect_eq(1000, cnt);

  list_free(list);
}

Test(list, push_front) {
  list *list = list_new(container_cmp_chars_chars, container_new_chars_chars,
                        container_delete_chars_chars);

  char *key1 = "key1";
  char *value1 = "value1";

  chars_chars data = {.key = key1, .value = value1};

  list_push_front(list, &data);

  chars_chars *data_in = list_front(list);

  cr_expect_str_eq(key1, data_in->key);
  cr_expect_str_eq(value1, data_in->value);

  list_free(list);
}
