#include <criterion/criterion.h>

#include "util/string.h"

Test(string, escape_html) {
  const char *str = "asd[sd";

  const char *out = escape_html(str);

  cr_assert_str_eq("asd&#91;sd", out);

  free((void *)out);
}

Test(string, string_all_char_in_symbols1) {
  const char *str = "0x1231";

  cr_assert(string_all_char_in_symbols(str, "0x123"));
}

Test(string, string_all_char_in_symbols2) {
  const char *str = "0x1231";

  cr_assert_not(string_all_char_in_symbols(str, "0123"));
}
