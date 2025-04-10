#include "mir_build.h"
#include "compiler/mir_build/lower_hir.h"
#include "compiler/mir_build/merge_bb.h"
#include "util/macro.h"

static inline int mir_ok(list_exception *exceptions, int ignore_errors) {
  return exceptions &&
         (!list_exception_count_by_level(exceptions, EXCEPTION_LEVEL_ERROR) ||
          ignore_errors);
}

mir_build_result mir_build(const hir *hir, const symbol_table *symbol_table,
                           const type_table *type_table, int ignore_errors) {
  UNUSED(symbol_table);

  mir_build_result result = {
      .mir        = NULL,
      .exceptions = list_exception_new(),
  };

  if (mir_ok(result.exceptions, ignore_errors)) {
    mir_lower_hir_result r = mir_lower_hir(hir, type_table);
    list_exception_extend(result.exceptions, r.exceptions);
    result.mir = r.mir;
  }

  if (mir_ok(result.exceptions, ignore_errors)) {
    mir_merge_bb_result r = mir_merge_bb(result.mir);
    list_exception_extend(result.exceptions, r.exceptions);
  }

  return result;
}
