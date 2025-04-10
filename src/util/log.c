#include "util/colors.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

int _debug(const char *fname, int lineno, const char *fxname, const char *fmt,
           ...) {
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_GRAY);
  }
  fprintf(stderr, "[debug %s:%d:%s]: ", fname, lineno, fxname);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_RESET);
  }
  fprintf(stderr, "\n");
  va_end(args);
  return 0;
}

int _info(const char *fname, int lineno, const char *fxname, const char *fmt,
          ...) {
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_CYAN);
  }
  fprintf(stderr, "[info %s:%d:%s]: ", fname, lineno, fxname);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_RESET);
  }
  fprintf(stderr, "\n");
  va_end(args);
  return 0;
}

int _warn(const char *fname, int lineno, const char *fxname, const char *fmt,
          ...) {
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_YELLOW);
  }
  fprintf(stderr, "[warn %s:%d:%s]: ", fname, lineno, fxname);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_RESET);
  }
  fprintf(stderr, "\n");
  va_end(args);
  return 0;
}

int _error(const char *fname, int lineno, const char *fxname, const char *fmt,
           ...) {
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_RED);
  }
  fprintf(stderr, "[error %s:%d:%s]: ", fname, lineno, fxname);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, ANSI_COLOR_RESET);
  }
  fprintf(stderr, "\n");
  va_end(args);
  return 0;
}
