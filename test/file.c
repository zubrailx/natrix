#include <criterion/criterion.h>

#include "util/file.h"

Test(file, dirname) {
  char *filename, *dir;

  filename = "io/input.txt";
  dir = dirname(filename);
  cr_expect_str_eq("io", dir);
  free(dir);

  filename = "io/";
  dir = dirname(filename);
  cr_expect_str_eq("io", dir);
  free(dir);

  filename = "io";
  dir = dirname(filename);
  cr_expect_str_eq("io", dir);
  free(dir);
}

Test(file, join) {
  char *filename, *dir, *path, *file;

  filename = "io/input.txt";
  dir = dirname(filename);
  file = "output.txt";
  path = join_paths(dir, file);
  cr_expect_str_eq("io/output.txt", path);
  free(dir);
  free(path);

  filename = "io/";
  dir = dirname(filename);
  file = "output.txt";
  path = join_paths(dir, file);
  cr_expect_str_eq("io/output.txt", path);
  free(dir);
  free(path);

  filename = ".";
  dir = dirname(filename);
  file = "output.txt";
  path = join_paths(dir, file);
  cr_expect_str_eq("./output.txt", path);
  free(dir);
  free(path);
}
