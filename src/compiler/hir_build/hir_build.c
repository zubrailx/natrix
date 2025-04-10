#include "hir_build.h"

#include "compiler/hir_build/bind_symbols.h"
#include "compiler/hir_build/bind_types.h"
#include "compiler/hir_build/expand_templates.h"
#include "compiler/hir_build/lower_ast.h"

static inline int hir_ok(list_exception *exceptions, int ignore_errors) {
  return exceptions &&
         (!list_exception_count_by_level(exceptions, EXCEPTION_LEVEL_ERROR) ||
          ignore_errors);
}

hir_build_result hir_build(const list_ast *asts, int ignore_errors) {
  hir_build_result result = {
      .hir          = NULL,
      .symbol_table = NULL,
      .type_table   = NULL,
      .exceptions   = list_exception_new(),
  };

  if (hir_ok(result.exceptions, ignore_errors)) {
    hir_lower_ast_result r = hir_lower_ast(asts);
    list_exception_extend(result.exceptions, r.exceptions);
    result.hir = r.hir;
  }

  if (hir_ok(result.exceptions, ignore_errors)) {
    hir_bind_types_result r = hir_bind_types(result.hir);
    list_exception_extend(result.exceptions, r.exceptions);
    result.type_table = r.type_table;
  }

  if (hir_ok(result.exceptions, ignore_errors)) {
    hir_expand_templates_result r =
        hir_expand_templates(result.hir, result.type_table);
    list_exception_extend(result.exceptions, r.exceptions);
  }

  if (hir_ok(result.exceptions, ignore_errors)) {
    hir_bind_symbols_result r = hir_bind_symbols(result.hir);
    list_exception_extend(result.exceptions, r.exceptions);
    result.symbol_table = r.symbol_table;
  }

  return result;
}
