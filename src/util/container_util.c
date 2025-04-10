#include "util/container_util.h"

#include "util/macro.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// CMP
int container_cmp_chars_chars(const void *_lsv, const void *_rsv) {
  const chars_chars *lsv = _lsv;
  const chars_chars *rsv = _rsv;
  return strcmp(lsv->key, rsv->key);
}

int container_cmp_chars(const void *_lsv, const void *_rsv) {
  const char *lsv = _lsv;
  const char *rsv = _rsv;
  return strcmp(lsv, rsv);
}

int container_cmp_false(const void *lsv, const void *rsv) {
  UNUSED(lsv);
  UNUSED(rsv);
  return 1;
}

int container_cmp_uint64(const void *_lsv, const void *_rsv) {
  const uint64_t lsv = (uint64_t)_lsv;
  const uint64_t rsv = (uint64_t)_rsv;
  if (lsv == rsv) {
    return 0;
  } else if (lsv < rsv) {
    return 1;
  } else {
    return -1;
  }
}

int container_cmp_ptr(const void *lsv, const void *rsv) {
  if (lsv == rsv) {
    return 0;
  } else if (lsv < rsv) {
    return 1;
  } else {
    return -1;
  }
}

// NEW
void *container_new_chars_chars(void *_data) {
  chars_chars *data = _data;

  chars_chars *new_data = MALLOC(chars_chars);
  new_data->key         = strdup(data->key);
  new_data->value       = strdup(data->value);

  return new_data;
}

void *container_new_chars(void *_data) { return strdup(_data); }

void *container_new_move(void *data) { return data; }

// DELETE
void container_delete_chars_chars(void *_data) {
  chars_chars *data = _data;
  free(data->key);
  free(data->value);
  free(data);
}

void container_delete_chars(void *_data) {
  if (_data) {
    free(_data);
  }
}

void container_delete_ptr(void *_data) {
  if (_data) {
    free(_data);
  }
}

void container_delete_false(void *_data) { UNUSED(_data); }

// HASH
uint64_t container_hash_chars(const void *_data) {
  const uint64_t pow  = 757;
  uint64_t       hash = 353;

  const char *kc = _data;
  while (*kc != '\0') {
    hash = hash * pow + *kc++;
  }

  return hash;
}

uint64_t container_hash_chars_chars(const void *_data) {
  const chars_chars *data = _data;
  return container_hash_chars(data->key);
}

uint64_t container_hash_uint64(const void *_data) {
  uint64_t ad = (uint64_t)_data;
  return (uint64_t)((13 * ad) ^ (ad >> 15));
}

uint64_t container_hash_ptr(const void *_data) {
  uintptr_t ad = (uintptr_t)_data;
  return (uint64_t)((13 * ad) ^ (ad >> 15));
}
