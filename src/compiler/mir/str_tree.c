#include "compiler/mir/mir.h"
#include "compiler/span/str.h"
#include "compiler/type_table/str.h"
#include "str.h"
#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"

static void strbuf_append_padding(strbuf *buffer, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    strbuf_append(buffer, MIR_STR_SPACE);
  }
}

static void strbuf_appendln(strbuf *buffer) { strbuf_append(buffer, "\n"); }

static void strbuf_append_clx(strbuf *buffer, const char *data, size_t pad) {
  if (pad) {
    strbuf_appendln(buffer);
    strbuf_append_padding(buffer, pad);
  }
  strbuf_append(buffer, data);
}

static void mir_str_symbol(strbuf *buffer, size_t pad,
                           const symbol_entry *symbol) {
  UNUSED(pad);
  char buf[64];

  strbuf_append(buffer, symbol->name);

  strbuf_append_f(buffer, buf, ", symbol_table(%p)", symbol);
}

static void mir_str_type(strbuf *buffer, size_t pad, const type_entry *type) {
  UNUSED(pad);
  char buf[64];

  if (type) {
    char *type_s = type_str(type->type);
    if (type_s) {
      strbuf_append(buffer, type_s);
      free(type_s);
    } else {
      strbuf_append(buffer, "(nil)");
    }
    strbuf_append_f(buffer, buf, ", type_table(%p)", type);
  } else {
    strbuf_append(buffer, "(nil)");
  }
}

static void mir_str_value(strbuf *buffer, size_t pad, const mir_value *value,
                          const char *prefix) {
  char buf[64];
  strbuf_append_clx(buffer, prefix, 0);
  strbuf_append(buffer, " ");
  strbuf_append_f(buffer, buf, "_%lu", value->id);
  strbuf_append(buffer, ": ");
  mir_str_type(buffer, pad, value->type_ref);
  if (value->symbol_ref) {
    strbuf_append(buffer, "  # ");
    mir_str_symbol(buffer, pad, value->symbol_ref);
  }
}

static void mir_str_stmt_args(strbuf *buffer, size_t pad,
                              const list_mir_value_ref *args) {
  UNUSED(pad);
  char buf[64];
  strbuf_append(buffer, "(");
  list_mir_value_ref_it it = list_mir_value_ref_begin(args);
  if (!END(it)) {
    strbuf_append_f(buffer, buf, "_%lu", GET(it)->id);
    for (NEXT(it); !END(it); NEXT(it)) {
      strbuf_append_f(buffer, buf, ", _%lu", GET(it)->id);
    }
  }
  strbuf_append(buffer, ")");
}

static void mir_str_debug(strbuf *buffer, size_t pad, const mir_debug *debug) {
  UNUSED(pad);
  char buf[64];

  if (debug->source_ref) {
    strbuf_append(buffer, "  # ");
    strbuf_append(buffer, debug->source_ref);
    strbuf_append_f(buffer, buf, ":%lu:%lu", debug->line, debug->pos);
  }
}

static void mir_str_stmt(strbuf *buffer, size_t pad, const mir_stmt *stmt) {
  UNUSED(pad);
  UNUSED(stmt);

  char buf[64];

  switch (stmt->kind) {

    case MIR_STMT_OP:
      if (stmt->op.ret) {
        strbuf_append_f(buffer, buf, "_%lu = ", stmt->op.ret->id);
      }
      strbuf_append(buffer, mir_stmt_enum_str(stmt->kind));
      strbuf_append(buffer, "_");
      strbuf_append(buffer, mir_stmt_op_enum_str(stmt->op.kind));
      mir_str_stmt_args(buffer, pad, stmt->op.args);
      break;

    case MIR_STMT_CALL:
      if (stmt->call.ret) {
        strbuf_append_f(buffer, buf, "_%lu = ", stmt->call.ret->id);
      }
      strbuf_append(buffer, stmt->call.sub->symbol_ref->name);
      mir_str_stmt_args(buffer, pad, stmt->op.args);
      break;

    case MIR_STMT_MEMBER:
    case MIR_STMT_MEMBER_REF:
      if (stmt->member.ret) {
        strbuf_append_f(buffer, buf, "_%lu = ", stmt->member.ret->id);
      }
      strbuf_append(buffer, mir_stmt_enum_str(stmt->kind));
      strbuf_append_f(buffer, buf, "(_%lu, ", stmt->member.obj->id);
      char *member_s = mir_lit_value_str(stmt->member.member);
      if (member_s) {
        strbuf_append(buffer, member_s);
        free(member_s);
      }
      strbuf_append(buffer, ")");
      break;

    case MIR_STMT_BUILTIN:
      if (stmt->builtin.ret) {
        strbuf_append_f(buffer, buf, "_%lu = ", stmt->builtin.ret->id);
      }
      strbuf_append(buffer, mir_stmt_enum_str(stmt->kind));
      strbuf_append(buffer, "_");
      strbuf_append(buffer, mir_stmt_builtin_enum_str(stmt->builtin.kind));
      if (stmt->builtin.type) {
        strbuf_append(buffer, "<");
        char *type_s = type_str(stmt->builtin.type->type);
        if (type_s) {
          strbuf_append(buffer, type_s);
          free(type_s);
        }
        strbuf_append(buffer, ">");
      }
      mir_str_stmt_args(buffer, pad, stmt->builtin.args);
      break;

    case MIR_STMT_ASSIGN:
      strbuf_append_f(buffer, buf, "assign(_%lu, ", stmt->assign.to->id);
      switch (stmt->assign.kind) {
        case MIR_STMT_ASSIGN_VALUE:
          strbuf_append_f(buffer, buf, "_%lu)", stmt->assign.from_value->id);
          break;
        case MIR_STMT_ASSIGN_LIT:
          strbuf_append_f(buffer, buf, "lit.%lu)", stmt->assign.from_lit->id);
          char *lit_s = mir_lit_value_str(stmt->assign.from_lit);
          if (lit_s) {
            strbuf_append(buffer, "  # ");
            strbuf_append(buffer, lit_s);
            free(lit_s);
          }
          if (stmt->assign.from_lit->type_ref) {
            char *type_s = type_str(stmt->assign.from_lit->type_ref->type);
            if (type_s) {
              if (lit_s) {
                strbuf_append(buffer, ", ");
              }
              strbuf_append(buffer, type_s);
              free(type_s);
            }
          }
          break;
        case MIR_STMT_ASSIGN_SUB:
          strbuf_append(buffer, stmt->assign.from_sub->symbol_ref->name);
          strbuf_append(buffer, ")");
          strbuf_append(buffer, "  # ");
          mir_str_symbol(buffer, pad, stmt->assign.from_sub->symbol_ref);
          break;
        default:
          warn("unhandled mir stmt assign kind %d %p", stmt->assign.kind, stmt);
          break;
      }
      break;

    default:
      warn("unexpected stmt kind %d %p", stmt->kind, stmt);
      break;
  }

  mir_str_debug(buffer, pad, &stmt->debug);
}

static void mir_str_bb(strbuf *buffer, size_t pad, const mir_subroutine *sub,
                       const mir_bb *bb) {
  char buf[64];
  strbuf_append_f(buffer, buf, "bb.%lu {", bb->id);
  pad += MIR_STR_TAB;

  strbuf_append_clx(buffer, "", pad);
  strbuf_append_f(buffer, buf, "hir_expr_trees_sz: %lu",
                  list_hir_expr_ref_size(bb->hir_exprs));

  if (!list_mir_stmt_empty(bb->stmts)) {
    strbuf_append_clx(buffer, "", pad);
    for (list_mir_stmt_it it = list_mir_stmt_begin(bb->stmts); !END(it);
         NEXT(it)) {
      const mir_stmt *stmt = GET(it);
      strbuf_append_clx(buffer, "", pad);
      mir_str_stmt(buffer, pad, stmt);
    }
  }

  strbuf_append_clx(buffer, "", pad);
  strbuf_append_clx(buffer, "", pad);
  mir_bb_enum bb_cond = mir_bb_get_cond(bb);
  switch (bb_cond) {
    case MIR_BB_COND:
      if (bb->jmp.cond_ref) {
        strbuf_append_f(buffer, buf, "cond(_%lu)", bb->jmp.cond_ref->id);
      } else {
        strbuf_append_f(buffer, buf, "cond(%p)", bb->jmp.cond_ref);
      }
      strbuf_append(buffer, " -> [");
      strbuf_append_f(buffer, buf, "je: bb.%lu", bb->jmp.je_ref->id);
      strbuf_append(buffer, ", ");
      strbuf_append_f(buffer, buf, "jz: bb.%lu", bb->jmp.jz_ref->id);
      strbuf_append(buffer, "]");
      break;
    case MIR_BB_NEXT:
      strbuf_append_f(buffer, buf, "jmp() -> bb.%lu", bb->jmp.next_ref->id);
      break;
    case MIR_BB_TERM:
      strbuf_append_f(buffer, buf, "return _%lu", sub->defined.ret->id);
      break;
    case MIR_BB_UNKNOWN:
    default:
      warn("unexpected bb kind %d %p", bb_cond, bb);
      break;
  }

  mir_str_debug(buffer, pad, &bb->jmp.debug);

  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "}", pad);
}

static void mir_str_subroutine(strbuf *buffer, size_t pad,
                               const mir_subroutine *sub) {
  char buf[64];

  if (!sub) {
    strbuf_append_clx(buffer, "(nil)", 0);
    return;
  }

  strbuf_append(buffer, sub->symbol_ref->name);
  strbuf_append(buffer, " {");

  // additional info
  strbuf_append(buffer, "  # ");

  strbuf_append_f(buffer, buf, "ptr(%p)", sub);

  char *span_s = span_str(sub->symbol_ref->span);
  if (span_s) {
    strbuf_append(buffer, ", ");
    strbuf_append(buffer, span_s);
    free(span_s);
  }

  pad += MIR_STR_TAB;

  strbuf_append_clx(buffer, "symbol: ", pad);
  mir_str_symbol(buffer, pad, sub->symbol_ref);

  strbuf_append_clx(buffer, "type: ", pad);
  mir_str_type(buffer, pad, sub->type_ref);

  strbuf_append_clx(buffer, "kind: ", pad);
  strbuf_append(buffer, mir_subroutine_enum_str(sub->kind));

  if (sub->spec != MIR_SUBROUTINE_SPEC_EMPTY) {
    strbuf_append_clx(buffer, "spec: ", pad);
    strbuf_append(buffer, mir_subroutine_spec_str(sub->spec));
  }

  switch (sub->kind) {
    case MIR_SUBROUTINE_DEFINED:
      strbuf_append_clx(buffer, "", pad);
      // ret
      strbuf_append_clx(buffer, "", pad);
      mir_str_value(buffer, pad, sub->defined.ret, "ret");
      // params
      for (list_mir_value_it it = list_mir_value_begin(sub->defined.params);
           !END(it); NEXT(it)) {
        const mir_value *param = GET(it);
        strbuf_append_clx(buffer, "", pad);
        mir_str_value(buffer, pad, param, "param");
      }
      // vars
      for (list_mir_value_it it = list_mir_value_begin(sub->defined.vars);
           !END(it); NEXT(it)) {
        const mir_value *param = GET(it);
        strbuf_append_clx(buffer, "", pad);
        mir_str_value(buffer, pad, param, "var");
      }
      // tmps
      for (list_mir_value_it it = list_mir_value_begin(sub->defined.tmps);
           !END(it); NEXT(it)) {
        const mir_value *param = GET(it);
        strbuf_append_clx(buffer, "", pad);
        mir_str_value(buffer, pad, param, "tmp");
      }
      // bbs
      strbuf_append_clx(buffer, "", pad);
      for (list_mir_bb_it it = list_mir_bb_begin(sub->defined.bbs); !END(it);
           NEXT(it)) {
        const mir_bb *bb = GET(it);
        strbuf_append_clx(buffer, "", pad);
        mir_str_bb(buffer, pad, sub, bb);
      }
      break;
    case MIR_SUBROUTINE_DECLARED:
      break;
    case MIR_SUBROUTINE_IMPORTED:
      strbuf_append_clx(buffer, "lib: ", pad);
      strbuf_append(buffer, sub->imported.lib);

      if (sub->imported.entry) {
        strbuf_append_clx(buffer, "entry: ", pad);
        strbuf_append(buffer, sub->imported.entry);
      }
      break;
    default:
      warn("unexpected subroutine kind %d %p", sub->kind, sub);
      break;
  }

  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "}", pad);
}

static void mir_str_class(strbuf *buffer, size_t pad, const mir_class *class) {
  char buf[64];

  if (!class) {
    strbuf_append_clx(buffer, "(nil)", 0);
    return;
  }

  strbuf_append_clx(buffer, "{", 0);
  pad += MIR_STR_TAB;

  strbuf_append_clx(buffer, "type: ", pad);
  mir_str_type(buffer, pad, class->type_ref);

  if (!list_mir_value_empty(class->fields)) {
    strbuf_append_clx(buffer, "", pad);
    for (list_mir_value_it it = list_mir_value_begin(class->fields); !END(it);
         NEXT(it)) {
      const mir_value *var = GET(it);

      strbuf_append_clx(buffer, "var ", pad);
      strbuf_append(buffer, var->symbol_ref->name);
      strbuf_append(buffer, ": ");
      mir_str_type(buffer, pad, var->type_ref);
      strbuf_append(buffer, "  # ");
      mir_str_symbol(buffer, pad, var->symbol_ref);
    }
  }

  if (!list_mir_subroutine_ref_empty(class->methods)) {
    strbuf_append_clx(buffer, "", pad);
    for (list_mir_subroutine_ref_it it =
             list_mir_subroutine_ref_begin(class->methods);
         !END(it); NEXT(it)) {
      mir_subroutine *sub = GET(it);
      strbuf_append_clx(buffer, "method ", pad);
      strbuf_append(buffer, sub->symbol_ref->name);
      strbuf_append_f(buffer, buf, "  # ptr(%p)", sub);
      strbuf_append(buffer, ", type: ");
      mir_str_type(buffer, pad, sub->type_ref);
    }
  }

  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "}", pad);
}

static void mir_str_lit(strbuf *buffer, size_t pad, const mir_lit *lit) {
  if (!lit) {
    strbuf_append_clx(buffer, "(nil)", 0);
    return;
  }

  char buf[64];

  strbuf_append_clx(buffer, "{ ", 0);
  pad += MIR_STR_TAB;

  strbuf_append_f(buffer, buf, "id: %lu", lit->id);

  char *value_s = mir_lit_value_str(lit);
  if (value_s) {
    strbuf_append(buffer, ", value: ");
    strbuf_append(buffer, value_s);
    free(value_s);
  }

  strbuf_append(buffer, ", type: ");
  mir_str_type(buffer, pad, lit->type_ref);

  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, " }", 0);
}

char *mir_str(const mir *mir) {
  strbuf *buffer = strbuf_new(0, 0);
  size_t  pad    = 0;

  strbuf_append_clx(buffer, "{", pad);
  pad += MIR_STR_TAB;

  // declared subroutines
  strbuf_append_clx(buffer, "DECLARED SUBROUTINES: [", pad);
  pad += MIR_STR_TAB;
  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->declared_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *subroutine = GET(it);
    strbuf_append_clx(buffer, "", pad);
    mir_str_subroutine(buffer, pad, subroutine);
  }
  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "]", pad);

  // imported subroutines
  strbuf_append_clx(buffer, "IMPORTED SUBROUTINES: [", pad);
  pad += MIR_STR_TAB;
  for (list_mir_subroutine_it it =
           list_mir_subroutine_begin(mir->imported_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *subroutine = GET(it);
    strbuf_append_clx(buffer, "", pad);
    mir_str_subroutine(buffer, pad, subroutine);
  }
  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "]", pad);

  // defined subroutines
  strbuf_append_clx(buffer, "DEFINED SUBROUTINES: [", pad);
  pad += MIR_STR_TAB;
  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->defined_subs);
       !END(it); NEXT(it)) {
    const mir_subroutine *subroutine = GET(it);
    strbuf_append_clx(buffer, "", pad);
    mir_str_subroutine(buffer, pad, subroutine);
  }
  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "]", pad);

  // methods
  strbuf_append_clx(buffer, "METHODS: [", pad);
  pad += MIR_STR_TAB;

  for (list_mir_subroutine_it it = list_mir_subroutine_begin(mir->methods);
       !END(it); NEXT(it)) {
    const mir_subroutine *subroutine = GET(it);
    strbuf_append_clx(buffer, "", pad);
    mir_str_subroutine(buffer, pad, subroutine);
  }
  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "]", pad);

  // classes
  strbuf_append_clx(buffer, "CLASSES: [", pad);
  pad += MIR_STR_TAB;

  for (list_mir_class_it it = list_mir_class_begin(mir->classes); !END(it);
       NEXT(it)) {
    const mir_class *class = GET(it);
    strbuf_append_clx(buffer, "", pad);
    mir_str_class(buffer, pad, class);
  }
  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "]", pad);

  // literals
  strbuf_append_clx(buffer, "LITERALS: [", pad);
  pad += MIR_STR_TAB;

  for (list_mir_lit_it it = list_mir_lit_begin(mir->literals); !END(it);
       NEXT(it)) {
    const mir_lit *lit = GET(it);
    strbuf_append_clx(buffer, "", pad);
    mir_str_lit(buffer, pad, lit);
  }
  pad -= MIR_STR_TAB;
  strbuf_append_clx(buffer, "]", pad);

  pad -= MIR_STR_TAB;
  strbuf_appendln(buffer);
  strbuf_append_clx(buffer, "}", pad);

  return strbuf_detach(buffer);
}
