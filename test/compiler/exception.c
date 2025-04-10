#include <criterion/criterion.h>

#include "compiler/exception/exception.h"

Test(exception, basic) {
  exception *exc =
      exception_new(EXCEPTION_LEVEL_ERROR, EXCEPTION_HIR, 0, "ordinary/file",
                    1000, 50, "exception", "ordinary error");
  char *msg = exception_str(exc, 0);
  free(msg);
  exception_free(exc);
}

Test(exception, basic2) {
  exception *exc =
      exception_new(EXCEPTION_LEVEL_ERROR, EXCEPTION_HIR, 0, "ordinary/file",
                    1000, 50, "exception", "ordinary error");
  char *msg = exception_str(exc, 0);
  free(msg);
  exception_free(exc);
}
