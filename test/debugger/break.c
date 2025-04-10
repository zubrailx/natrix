#include <criterion/criterion.h>

#include "debugger/ctx/ctx_data.h"

#include <stdint.h>

Test(debugger, break_1) {
  hashset_ctx_breakpoint *breaks = hashset_ctx_breakpoint_new();

  ctx_breakpoint *b1 = ctx_breakpoint_new(1, 1, UINT64_MAX, UINT64_MAX);
  ctx_breakpoint *b2 = ctx_breakpoint_new(2, 1, UINT64_MAX, UINT64_MAX);

  hashset_ctx_breakpoint_insert(breaks, b1);

  hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_find(breaks, b2);
  cr_expect_eq(it.end(&it), false);

  ctx_breakpoint_free(b2);
  hashset_ctx_breakpoint_free(breaks);
}
