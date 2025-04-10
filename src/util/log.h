#pragma once

int _debug(const char *fname, int lineno, const char *fxname, const char *fmt,
           ...);

int _info(const char *fname, int lineno, const char *fxname, const char *fmt,
          ...);

int _warn(const char *fname, int lineno, const char *fxname, const char *fmt,
          ...);

int _error(const char *fname, int lineno, const char *fxname, const char *fmt,
           ...);

#ifndef NDEBUG

#define debug(fmt, ...)                                                        \
  (_debug(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__))
#define info(fmt, ...)                                                         \
  (_info(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__))
#define warn(fmt, ...)                                                         \
  (_warn(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__))
#define error(fmt, ...)                                                        \
  (_error(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__))

#else

#define debug(fmt, ...)
#define info(fmt, ...)
#define warn(fmt, ...)
#define error(fmt, ...)

#endif
