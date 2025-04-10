#include "util/file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

char *join_paths(const char *dir, const char *file) {
  size_t len_dir  = strlen(dir);
  size_t len_file = strlen(file);

  char *result = malloc(len_dir + len_file + 2);
  if (!result) {
    perror("malloc failed");
    return NULL;
  }

#ifdef _WIN32
  sprintf(result, "%s\\%s", dir, file);
#else
  sprintf(result, "%s/%s", dir, file);
#endif

  return result;
}

int dir_create(const char *path) {
  struct stat st = {0};

  if (stat(path, &st) == -1) {
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
      perror("mkdir failed");
      return -1;
    }
  }

  return 0;
}

int dir_create_p(const char *dir) {
  char *parent = dirname(dir);
  if (strcmp(dir, parent)) {
    dir_create_p(parent);
  }
  free(parent);
  return dir_create(dir);
}

char *dirname(const char *file_path) {
  char *dir_path = strdup(file_path);
  if (!dir_path) {
    perror("strdup failed");
    return NULL;
  }

  char *last_separator = strrchr(dir_path, '/');
#ifdef _WIN32
  if (!last_separator)
    last_separator = strrchr(dir_path, '\\');
#endif

  if (last_separator) {
    *last_separator = '\0';
  }

  return dir_path;
}

char *read_file(const char *path) {
  char *buffer = 0;
  long  length;
  FILE *f = fopen(path, "rb");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length + 1);
    if (buffer) {
      fread(buffer, 1, length, f);
      buffer[length] = '\0';
    }
    fclose(f);
  }

  return buffer;
}
