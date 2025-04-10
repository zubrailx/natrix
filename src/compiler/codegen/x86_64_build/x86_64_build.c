#include "x86_64_build.h"

#include "compiler/codegen/x86_64_build/debug_emit/debug_emit.h"
#include "compiler/codegen/x86_64_build/inst/inst.h"

static inline int cg_ok(list_exception *exceptions, int ignore_errors) {
  return exceptions &&
         (!list_exception_count_by_level(exceptions, EXCEPTION_LEVEL_ERROR) ||
          ignore_errors);
}

cg_x86_64_build_result cg_x86_64_build(const mir *mir, int ignore_errors) {
  cg_x86_64_build_result result = {
      .code       = cg_x86_64_new(),
      .exceptions = list_exception_new(),
  };

  cg_debug *debug = NULL;

  if (cg_ok(result.exceptions, ignore_errors)) {
    cg_inst_result r = cg_inst(result.code, mir);
    list_exception_extend(result.exceptions, r.exceptions);
    debug = r.debug;
  }

  if (cg_ok(result.exceptions, ignore_errors)) {
    cg_debug_emit_result r = cg_debug_emit(result.code, debug);
    list_exception_extend(result.exceptions, r.exceptions);
  }

  cg_debug_free(debug);

  return result;
}
