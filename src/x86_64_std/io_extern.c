#include "io_extern.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char *trim_end(char *str) {
  if (*str) {
    char *end = str + strlen(str) - 1;

    while (*end && isspace((unsigned char)*end)) {
      end--;
    }
    *(end + 1) = '\0';
  }
  return str;
}

uint8_t std_read_bool() {
  char buf[16];
  scanf("%15s", buf);

  if (strcmp(buf, "true")) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t std_read_byte() {
  uint8_t v;
  scanf("%hhu", &v);
  return v;
}

uint8_t std_read_char() {
  uint8_t v;
  scanf(" %c", &v);
  return v;
}

int32_t std_read_int() {
  int32_t v;
  scanf("%d", &v);
  return v;
}

uint32_t std_read_uint() {
  uint32_t v;
  scanf("%u", &v);
  return v;
}

int64_t std_read_long() {
  int64_t v;
  scanf("%ld", &v);
  return v;
}

uint64_t std_read_ulong() {
  uint64_t v;
  scanf("%lu", &v);
  return v;
}

uint8_t *std_read_string() {
  char  *line = NULL;
  size_t len  = 0;
  getline(&line, &len, stdin);
  if (line == NULL) {
    return NULL;
  }
  line = trim_end(line);
  return (uint8_t *)line;
}

void std_write_bool(uint8_t v) {
  if (v) {
    printf("true");
  } else {
    printf("false");
  }
}

void std_write_byte(uint8_t v) { printf("%hhu", v); }

void std_write_char(uint8_t v) { printf("%c", v); }

void std_write_int(int32_t v) { printf("%d", v); }

void std_write_uint(uint32_t v) { printf("%u", v); }

void std_write_long(int64_t v) { printf("%ld", v); }

void std_write_ulong(uint64_t v) { printf("%lu", v); }

void std_write_string(const uint8_t *v) { printf("%s", v); }
