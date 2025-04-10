#include "mir.h"
#include "util/log.h"
#include "util/macro.h"

static void mir_debug_init(mir_debug *debug, const char *source_ref,
                           uint64_t line, uint64_t pos) {
  debug->source_ref = source_ref;
  debug->line       = line;
  debug->pos        = pos;
}

static void mir_debug_deinit(mir_debug *debug) { debug->source_ref = NULL; }

span *mir_debug_to_span(const mir_debug *debug) {
  if (!debug || !debug->source_ref) {
    return NULL;
  }
  return span_new(debug->source_ref, debug->line, debug->line, debug->pos,
                  debug->pos);
}

mir_value *mir_value_new(size_t id, const symbol_entry *symbol_ref,
                         const type_entry *type_ref) {
  mir_value *self  = MALLOC(mir_value);
  self->id         = id;
  self->symbol_ref = symbol_ref;
  self->type_ref   = type_ref;
  return self;
}

void mir_value_free(mir_value *self) {
  if (self) {
    free(self);
  }
}

mir_lit *mir_lit_new(size_t id, const type_entry *type_ref,
                     mir_lit_value value) {
  mir_lit *self  = MALLOC(mir_lit);
  self->id       = id;
  self->type_ref = type_ref;
  self->value    = value;
  return self;
}

void mir_lit_free(mir_lit *self) {
  if (self) {
    const type_base *type = self->type_ref ? self->type_ref->type : NULL;

    if (type) {
      switch (type->kind) {
        case TYPE_PRIMITIVE: {
          type_primitive *primitive = (typeof(primitive))type;
          switch (primitive->type) {
            case TYPE_PRIMITIVE_STRING:
              if (self->value.v_str) {
                free(self->value.v_str);
              }
              break;
            case TYPE_PRIMITIVE_BOOL:
            case TYPE_PRIMITIVE_BYTE:
            case TYPE_PRIMITIVE_INT:
            case TYPE_PRIMITIVE_UINT:
            case TYPE_PRIMITIVE_LONG:
            case TYPE_PRIMITIVE_ULONG:
            case TYPE_PRIMITIVE_CHAR:
            case TYPE_PRIMITIVE_VOID:
            case TYPE_PRIMITIVE_ANY:
              break;
          }
          break;
        }
        case TYPE_ARRAY:
        case TYPE_CALLABLE:
        case TYPE_CLASS_T:
        case TYPE_TYPENAME:
        case TYPE_MONO:
        default:
          error("unsupported literal kind %d %p", type->kind, self);
          break;
      }
    }
    free(self);
  }
}

// stmt
static void mir_stmt_init(mir_stmt *stmt, mir_stmt_enum kind) {
  stmt->kind = kind;
  mir_debug_init(&stmt->debug, NULL, 0, 0);
}

static void mir_stmt_deinit(mir_stmt *stmt) { mir_debug_deinit(&stmt->debug); }

mir_stmt *mir_stmt_new_op(mir_stmt_op_enum kind, mir_value *ret,
                          list_mir_value_ref *args) {
  mir_stmt *self = MALLOC(mir_stmt);
  mir_stmt_init(self, MIR_STMT_OP);
  self->op.kind = kind;
  self->op.ret  = ret;
  self->op.args = args;
  return self;
}

mir_stmt *mir_stmt_new_call(mir_value *ret, mir_subroutine *sub,
                            list_mir_value_ref *args) {
  mir_stmt *self = MALLOC(mir_stmt);
  mir_stmt_init(self, MIR_STMT_CALL);
  self->call.ret  = ret;
  self->call.sub  = sub;
  self->call.args = args;
  return self;
}

mir_stmt *mir_stmt_new_member(mir_value *ret, mir_value *obj, mir_lit *member) {
  mir_stmt *self = MALLOC(mir_stmt);
  mir_stmt_init(self, MIR_STMT_MEMBER);
  self->member.ret    = ret;
  self->member.obj    = obj;
  self->member.member = member;
  return self;
}

mir_stmt *mir_stmt_new_member_ref(mir_value *ret, mir_value *obj,
                                  mir_lit *member) {
  mir_stmt *self = MALLOC(mir_stmt);
  mir_stmt_init(self, MIR_STMT_MEMBER_REF);
  self->member.ret    = ret;
  self->member.obj    = obj;
  self->member.member = member;
  return self;
}

mir_stmt *mir_stmt_new_builtin(mir_stmt_builtin_enum kind, mir_value *ret,
                               type_entry *type, list_mir_value_ref *args) {
  mir_stmt *self     = MALLOC(mir_stmt);
  self->kind         = MIR_STMT_BUILTIN;
  self->builtin.kind = kind;
  self->builtin.ret  = ret;
  self->builtin.type = type;
  self->builtin.args = args;
  return self;
}

mir_stmt *mir_stmt_new_assign(mir_stmt_assign_enum kind, mir_value *to,
                              void *from) {
  mir_stmt *self = MALLOC(mir_stmt);
  mir_stmt_init(self, MIR_STMT_ASSIGN);
  self->assign.kind = kind;
  switch (kind) {
    case MIR_STMT_ASSIGN_LIT:
      self->assign.from_lit = from;
      break;
    case MIR_STMT_ASSIGN_VALUE:
      self->assign.from_value = from;
      break;
    case MIR_STMT_ASSIGN_SUB:
      self->assign.from_sub = from;
      break;
    default:
      error("unhandled assign kind %d", kind);
      break;
  }
  self->assign.to = to;
  return self;
}

void mir_stmt_debug_init_span(mir_stmt *stmt, const span *span) {
  if (span) {
    mir_debug_init(&stmt->debug, span->source_ref, span->line_start,
                   span->pos_start);
  }
}

void mir_stmt_free(mir_stmt *self) {
  if (self) {
    switch (self->kind) {
      case MIR_STMT_OP:
        list_mir_value_ref_free(self->op.args);
        break;
      case MIR_STMT_CALL:
        list_mir_value_ref_free(self->call.args);
        break;
      case MIR_STMT_MEMBER:
      case MIR_STMT_MEMBER_REF:
        break;
      case MIR_STMT_BUILTIN:
        list_mir_value_ref_free(self->builtin.args);
        break;
      case MIR_STMT_ASSIGN:
        break;
      default:
        error("unknown stmt kind %d %p", self->kind, self);
        break;
    }
    mir_stmt_deinit(self);
    free(self);
  }
}

mir_bb *mir_bb_new(size_t id, list_mir_stmt *stmts, mir_value *jmp_cond_ref,
                   mir_bb *jmp_je_ref, mir_bb *jmp_jz_ref,
                   list_hir_expr_ref *hir_exprs) {
  mir_bb *self       = MALLOC(mir_bb);
  self->id           = id;
  self->stmts        = stmts;
  self->jmp.cond_ref = jmp_cond_ref;
  self->jmp.je_ref   = jmp_je_ref;
  self->jmp.jz_ref   = jmp_jz_ref;
  mir_debug_init(&self->jmp.debug, NULL, 0, 0);
  self->hir_exprs = hir_exprs;
  return self;
}

void mir_bb_jmp_debug_init_span(mir_bb *bb, const span *span) {
  if (span) {
    mir_debug_init(&bb->jmp.debug, span->source_ref, span->line_start,
                   span->pos_start);
  }
}

void mir_bb_free(mir_bb *self) {
  if (self) {
    list_mir_stmt_free(self->stmts);
    list_hir_expr_ref_free(self->hir_exprs);
    mir_debug_deinit(&self->jmp.debug);
    free(self);
  }
}

mir_bb_enum mir_bb_get_cond(const mir_bb *self) {
  if (!self) {
    return MIR_BB_UNKNOWN;
  }
  if (self->jmp.je_ref) {
    return MIR_BB_COND;
  }
  if (self->jmp.next_ref) {
    return MIR_BB_NEXT;
  }
  return MIR_BB_TERM;
}

mir_subroutine *mir_subroutine_new_defined(
    const symbol_entry *symbol_ref, const type_entry *type_ref,
    mir_subroutine_spec spec, mir_value *ret, list_mir_value *params,
    list_mir_value *vars, list_mir_value *tmps, list_mir_bb *bbs) {
  mir_subroutine *self = MALLOC(mir_subroutine);
  self->kind           = MIR_SUBROUTINE_DEFINED;
  self->symbol_ref     = symbol_ref;
  self->type_ref       = type_ref;
  self->spec           = spec;
  self->defined.ret    = ret;
  self->defined.params = params;
  self->defined.vars   = vars;
  self->defined.tmps   = tmps;
  self->defined.bbs    = bbs;
  return self;
}

mir_subroutine *mir_subroutine_new_declared(const symbol_entry *symbol_ref,
                                            const type_entry   *type_ref,
                                            mir_subroutine_spec spec) {
  mir_subroutine *self = MALLOC(mir_subroutine);
  self->kind           = MIR_SUBROUTINE_DECLARED;
  self->symbol_ref     = symbol_ref;
  self->type_ref       = type_ref;
  self->spec           = spec;
  return self;
}

mir_subroutine *mir_subroutine_new_imported(const symbol_entry *symbol_ref,
                                            const type_entry   *type_ref,
                                            mir_subroutine_spec spec, char *lib,
                                            char *entry) {
  mir_subroutine *self = MALLOC(mir_subroutine);
  self->kind           = MIR_SUBROUTINE_IMPORTED;
  self->symbol_ref     = symbol_ref;
  self->type_ref       = type_ref;
  self->spec           = spec;
  self->imported.lib   = lib;
  self->imported.entry = entry;
  return self;
}

void mir_subroutine_free(mir_subroutine *self) {
  if (self) {
    switch (self->kind) {
      case MIR_SUBROUTINE_DEFINED:
        mir_value_free(self->defined.ret);
        list_mir_value_free(self->defined.params);
        list_mir_value_free(self->defined.vars);
        list_mir_value_free(self->defined.tmps);
        list_mir_bb_free(self->defined.bbs);
        break;
      case MIR_SUBROUTINE_DECLARED:
        break;
      case MIR_SUBROUTINE_IMPORTED:
        if (self->imported.lib) {
          free(self->imported.lib);
        }
        if (self->imported.entry) {
          free(self->imported.entry);
        }
        break;
      default:
        error("unknown subroutine kind %d %p", self->kind, self);
        break;
    }
    free(self);
  }
}

mir_class *mir_class_new(const type_entry *type_ref, list_mir_value *fields,
                         list_mir_subroutine_ref *methods) {
  mir_class *self = MALLOC(mir_class);
  self->type_ref  = type_ref;
  self->fields    = fields;
  self->methods   = methods;
  return self;
}

void mir_class_free(mir_class *self) {
  if (self) {
    list_mir_value_free(self->fields);
    list_mir_subroutine_ref_free(self->methods);
    free(self);
  }
}

mir *mir_new() {
  mir *self           = MALLOC(mir);
  self->defined_subs  = list_mir_subroutine_new();
  self->declared_subs = list_mir_subroutine_new();
  self->imported_subs = list_mir_subroutine_new();
  self->methods       = list_mir_subroutine_new();
  self->classes       = list_mir_class_new();
  self->literals      = list_mir_lit_new();
  return self;
}

void mir_free(mir *self) {
  if (self) {
    list_mir_subroutine_free(self->defined_subs);
    list_mir_subroutine_free(self->declared_subs);
    list_mir_subroutine_free(self->imported_subs);
    list_mir_subroutine_free(self->methods);
    list_mir_class_free(self->classes);
    list_mir_lit_free(self->literals);
    free(self);
  }
}
