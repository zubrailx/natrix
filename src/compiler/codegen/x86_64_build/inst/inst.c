#include "inst.h"
#include "compiler/codegen/exception.h"
#include "compiler/codegen/x86_64/x86_64.h"
#include "util/macro.h"
#include <string.h>

cg_inst_result cg_inst(cg_x86_64 *code, const mir *mir) {
  cg_inst_result result = {
      .debug      = cg_debug_new(CG_CTX_DEBUG_LEVEL_ENABLED),
      .exceptions = list_exception_new(),
  };

  cg_ctx ctx;
  cg_ctx_init(&ctx, code, result.debug, result.exceptions);

  // core
  cg_inst_core(&ctx);

  // TODO: imported subroutines
  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->imported_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *sub = GET(it);
    cg_exception_add_error(ctx.exceptions, EXCEPTION_CG_UNSUPPORTED_IMPORTED,
                           sub->symbol_ref->span,
                           "TODO: subroutine '%s' can't be instantiated",
                           sub->symbol_ref->name);
  }

  // mir_sym index declared_subs
  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->declared_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *sub_decl = GET(it);
    const char           *sub_sym  = sub_decl->symbol_ref->name;

    cg_ctx_mir_sym_emplace_sub(&ctx, sub_decl, sub_sym);
  }

  // mir_sym index defined_subs
  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->defined_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *sub_def = GET(it);
    const char           *sub_sym = sub_def->symbol_ref->name;

    cg_ctx_mir_sym_emplace_sub(&ctx, sub_def, sub_sym);
  }

  // mir_sym index class initializers
  list_cg_class_sym *class_inits = list_cg_class_sym_new();

  for (list_mir_class_it it = list_mir_class_begin(mir->classes); !END(it);
       NEXT(it)) {
    const mir_class *class = GET(it);
    const type_base *type  = class->type_ref->type;
    char            *sym;

    if (cg_ctx_type_sym_init_find_class(&ctx, (const type_mono *)type)) {
      continue;
    }

    sym = cg_ctx_type_sym_init_emplace_class(&ctx, (const type_mono *)type);
    list_cg_class_sym_push_back(class_inits, cg_class_sym_new(class, sym));
  }

  // mir_sym index class methods
  list_cg_method_sym *class_methods = list_cg_method_sym_new();

  for (list_mir_class_it it = list_mir_class_begin(mir->classes); !END(it);
       NEXT(it)) {
    const mir_class *class = GET(it);

    for (list_mir_subroutine_ref_it it_s =
             list_mir_subroutine_ref_begin(class->methods);
         !END(it_s); NEXT(it_s)) {
      const mir_subroutine *method = GET(it_s);
      char                 *sym;

      if (cg_ctx_mir_sym_find_sub(&ctx, method)) {
        continue;
      }

      sym = cg_ctx_mir_sym_emplace_method(&ctx, method);
      list_cg_method_sym_push_back(class_methods,
                                   cg_method_sym_new(method, sym));
    }
  }

  // validate declared_subs
  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->declared_subs);
       !END(it); NEXT(it)) {
    cg_inst_sub_decl_check(&ctx, GET(it));
  }

  // declare declared_subs extern
  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->declared_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *sub_decl = GET(it);
    const char           *sub_sym  = cg_ctx_mir_sym_find_sub(&ctx, sub_decl);

    cg_ctx_text_push_back(&ctx, cg_x86_64_symbol_new_extern(strdup(sub_sym)));
  }

  // declare defined_subs global
  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->defined_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *sub_def = GET(it);
    const char           *sub_sym = cg_ctx_mir_sym_find_sub(&ctx, sub_def);

    cg_ctx_text_push_back(&ctx, cg_x86_64_symbol_new_global(strdup(sub_sym)));
  }

  // instantiate defined_subs
  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->defined_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *sub     = GET(it);
    const char           *sub_sym = cg_ctx_mir_sym_find_sub(&ctx, sub);

    int is_main = !strcmp(sub->symbol_ref->name, "main");
    if (is_main) {
      cg_inst_sub_main(&ctx, sub, strdup(sub_sym));
    } else {
      cg_inst_sub_def(&ctx, sub, strdup(sub_sym));
    }
  }

  // instantiate class initializers and methods
  cg_inst_class_inits(&ctx, class_inits);
  cg_inst_class_methods(&ctx, class_methods);

  cg_ctx_deinit(&ctx);

  return result;
}
