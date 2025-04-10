#include "util/string.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *escape_html(const char *str) {
  size_t len     = strlen(str);
  size_t new_len = len * 6; // worst case

  char *out = (char *)malloc(new_len + 1);
  if (!out) {
    return NULL;
  }

  char *end = out;
  for (size_t i = 0; i < len; ++i) {
    const char *after = NULL;

    switch (str[i]) {
      case '&':
        after = "&amp;";
        break;
      case '<':
        after = "&lt;";
        break;
      case '>':
        after = "&gt;";
        break;
      case '"':
        after = "&quot;";
        break;
      case '\'':
        after = "&#39;";
        break;
      case '[':
        after = "&#91;";
        break;
      case ']':
        after = "&#93;";
        break;
    }

    if (after) {
      end = stpcpy(end, after);
    } else {
      end[0] = str[i];
      end[1] = '\0';
      end += 1;
    }
  }

  return out;
}

int string_all_char_in_symbols(const char *str, const char *symbols) {
  size_t n = strlen(str);
  size_t m = strlen(symbols);
  for (size_t i = 0; i < n; ++i) {
    int found = 0;
    for (size_t j = 0; j < m; ++j) {
      if (str[i] == symbols[j]) {
        found = 1;
        break;
      }
    }
    if (!found) {
      return 0;
    }
  }
  return 1;
}

char *trim_end(char *str_m) {
  if (*str_m) {
    char *end = str_m + strlen(str_m) - 1;

    while (*end && isspace((unsigned char)*end)) {
      end--;
    }
    *(end + 1) = '\0';
  }
  return str_m;
}

char *trim_start(char *str_m) {
  if (*str_m) {
    while (*str_m && isspace((unsigned char)*str_m)) {
      ++str_m;
    }
  }
  return str_m;
}

char *trim(char *str_m) {
  char *cur   = trim_start(str_m);
  char *n_str = strdup(cur);
  free(str_m);
  return trim_end(n_str);
}
