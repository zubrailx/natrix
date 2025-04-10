#include "x86_64_emit.h"
#include "compiler/codegen/exception.h"
#include "compiler/codegen/x86_64_emit/emit.h"

cg_x86_64_emit_result cg_x86_64_emit(const cg_x86_64      *code,
                                     cg_x86_64_emit_format format) {
  cg_x86_64_emit_result result = {
      .asm_str    = NULL,
      .exceptions = list_exception_new(),
  };

  cg_x86_64_emit_ctx ctx = {
      .exceptions = result.exceptions,
  };

  if (format == CG_X86_64_EMIT_FORMAT_GAS) {
    result.asm_str = cg_x86_64_emit_gas(&ctx, code);
  } else {
    cg_exception_add_error(result.exceptions,
                           EXCEPTION_CG_UNEXPECTED_EMIT_FORMAT, NULL,
                           "only GAS format is supported");
  }

  return result;
}
