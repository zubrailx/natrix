#include <criterion/criterion.h>
#include <string.h>

Test(strcpy, test) {
  char        buf[50];
  const char *a = "four";

  char *end = stpcpy(buf, a);

  cr_expect_eq(buf + 4, end);

  end = stpcpy(end, a);
  end = stpcpy(end, a);
  end = stpcpy(end, a);

  cr_expect_str_eq("fourfourfourfour", buf);
}
