#include <criterion/criterion.h>

#include <stdio.h>

#include "util/container_util.h"
#include "util/hashset.h"

HASHSET_DECLARE_STATIC_INLINE(hashset_char_char, chars_chars,
                              container_cmp_chars_chars,
                              container_new_chars_chars,
                              container_delete_chars_chars,
                              container_hash_chars_chars);

Test(hashset, create_delete) {
  hashset *hashset =
      hashset_new(container_cmp_chars_chars, container_new_chars_chars,
                  container_delete_chars_chars, container_hash_chars_chars);

  hashset_free(hashset);
}

Test(hashset, insert) {
  hashset *hashset =
      hashset_new(container_cmp_chars_chars, container_new_chars_chars,
                  container_delete_chars_chars, container_hash_chars_chars);

  char key[10];
  char value[10];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    hashset_insert(hashset, &data);
  }

  hashset_free(hashset);
}

Test(hashset, iterate_1000) {
  hashset *hashset =
      hashset_new(container_cmp_chars_chars, container_new_chars_chars,
                  container_delete_chars_chars, container_hash_chars_chars);

  char key[10];
  char value[10];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    hashset_insert(hashset, &data);
  }

  size_t cnt = 0;
  for (hashset_it it = hashset_begin(hashset); !hashset_it_end(&it);
       hashset_it_next(&it)) {
    cnt += 1;
  }

  cr_expect_eq(1000, cnt);

  hashset_free(hashset);
}

Test(hashset, iterate_empty) {
  hashset *hashset =
      hashset_new(container_cmp_chars_chars, container_new_chars_chars,
                  container_delete_chars_chars, container_hash_chars_chars);

  size_t cnt = 0;
  for (hashset_it it = hashset_begin(hashset); !hashset_it_end(&it);
       hashset_it_next(&it)) {
    hashset_it_get(&it);
    cnt += 1;
  }

  cr_expect_eq(0, cnt);

  hashset_free(hashset);
}

Test(hashset, char_char) {
  hashset_char_char *hashset = hashset_char_char_new();

  char key[10];
  char value[10];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    hashset_char_char_insert(hashset, &data);
  }

  size_t cnt = 0;
  for (hashset_char_char_it it = hashset_char_char_begin(hashset);
       !hashset_char_char_it_end(&it); hashset_char_char_it_next(&it)) {
    cnt += 1;
  }

  cr_expect_eq(1000, cnt);

  hashset_char_char_free(hashset);
}

Test(hashset, char_char_it) {
  hashset_char_char *hashset = hashset_char_char_new();

  char key[10];
  char value[10];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    hashset_char_char_insert(hashset, &data);
  }

  size_t cnt = 0;
  for (hashset_char_char_it it = hashset_char_char_begin(hashset); !it.end(&it);
       it.next(&it)) {
    cnt += 1;
  }

  cr_expect_eq(1000, cnt);

  hashset_char_char_free(hashset);
}

Test(hashset, char_char_erase) {
  hashset_char_char *hashset = hashset_char_char_new();

  char key[10];
  char value[10];

  chars_chars data = {.key = key, .value = value};

  for (int i = 0; i < 1000; ++i) {
    sprintf(key, "%d", i);
    sprintf(value, "%d", i + 1000);
    hashset_char_char_insert(hashset, &data);
  }

  hashset_char_char_free(hashset);
}
