#pragma once

#include "util/container.h"

struct chars_chars {
  char *key;
  char *value;
};

typedef struct chars_chars chars_chars;

container_f_cmp container_cmp_chars_chars;
container_f_cmp container_cmp_chars;
container_f_cmp container_cmp_false;
container_f_cmp container_cmp_uint64;
container_f_cmp container_cmp_ptr;

container_f_new container_new_chars_chars;
container_f_new container_new_chars;
container_f_new container_new_move;

container_f_delete container_delete_chars_chars;
container_f_delete container_delete_chars;
container_f_delete container_delete_false;
container_f_delete container_delete_ptr;

container_f_hash container_hash_chars;
container_f_hash container_hash_chars_chars;
container_f_hash container_hash_uint64;
container_f_hash container_hash_ptr;
