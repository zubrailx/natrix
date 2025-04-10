#include "lower_hir.h"
#include "compiler/exception/exception.h"
#include "compiler/exception/list.h"
#include "compiler/hir/str.h"
#include "compiler/mir/mir.h"
#include "compiler/mir_build/exception.h"
#include "compiler/type_table/str.h"
#include "util/hashset.h"
#include "util/log.h"
#include "util/macro.h"
#include <string.h>

// - resolve global subroutines (not methods)
// - resolve mir classes with dummy mir_subroutines
// - map[hir_subroutine] -> mir_subroutine
// - instantiate mir_subroutines that are actually used

// TYPE_REF
static inline int container_cmp_type_ref(const void *lsv, const void *rsv) {
  const type_base *l = lsv;
  const type_base *r = rsv;
  return type_cmp(l, r);
}
static inline uint64_t container_hash_type_ref(const void *lsv) {
  const type_base *l = lsv;
  return type_hash(l);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_type_ref, type_base,
                              container_cmp_type_ref, container_new_move,
                              container_delete_false, container_hash_type_ref);

// SYMBOL_ENTRY (compares by symbol name)
static inline int container_cmp_symbol_entry(const void *lsv, const void *rsv) {
  const symbol_entry *l = lsv;
  const symbol_entry *r = rsv;
  return container_cmp_chars(l->name, r->name);
}
static inline uint64_t container_hash_symbol_entry(const void *lsv) {
  const symbol_entry *l = lsv;
  return container_hash_chars(l->name);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_symbol_entry, symbol_entry,
                              container_cmp_symbol_entry, container_new_move,
                              container_delete_false,
                              container_hash_symbol_entry);

// SYM_ENTRY
typedef struct mir_sym_entry_struct {
  const symbol_entry *symbol_ref;
  void               *entry_ref;
} mir_sym_entry;

static mir_sym_entry *mir_sym_entry_new(const symbol_entry *symbol_ref,
                                        void               *entry_ref) {
  mir_sym_entry *self = MALLOC(mir_sym_entry);
  self->symbol_ref    = symbol_ref;
  self->entry_ref     = entry_ref;
  return self;
}

static void mir_sym_entry_free(mir_sym_entry *self) {
  if (self) {
    free(self);
  }
}

static inline int container_cmp_mir_sym_entry(const void *lsv,
                                              const void *rsv) {
  const mir_sym_entry *l = lsv;
  const mir_sym_entry *r = rsv;
  return container_cmp_ptr(l->symbol_ref, r->symbol_ref);
}
static inline uint64_t container_hash_mir_sym_entry(const void *lsv) {
  const mir_sym_entry *l = lsv;
  return container_hash_ptr(l->symbol_ref);
}
static inline void container_delete_mir_sym_entry(void *self) {
  mir_sym_entry_free(self);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_mir_sym_entry, mir_sym_entry,
                              container_cmp_mir_sym_entry, container_new_move,
                              container_delete_mir_sym_entry,
                              container_hash_mir_sym_entry);

// BB_SEQ
typedef struct mir_bb_seq_struct {
  mir_bb *first;
  mir_bb *last;
} mir_bb_seq;

LIST_DECLARE_STATIC_INLINE(list_mir_scope_ref, mir_bb_seq, container_cmp_false,
                           container_new_move, container_delete_false);

// TYPE_ENTRY
typedef struct mir_type_entry_struct {
  const type_entry *type_ref;
  void             *entry_ref;
} mir_type_entry;

static mir_type_entry *mir_type_entry_new(const type_entry *type_ref,
                                          void             *entry_ref) {
  mir_type_entry *self = MALLOC(mir_type_entry);
  self->type_ref       = type_ref;
  self->entry_ref      = entry_ref;
  return self;
}

static void mir_type_entry_free(mir_type_entry *self) {
  if (self) {
    free(self);
  }
}

static inline int container_cmp_mir_type_entry(const void *lsv,
                                               const void *rsv) {
  const mir_type_entry *l = lsv;
  const mir_type_entry *r = rsv;
  return container_cmp_ptr(l->type_ref, r->type_ref);
}
static inline uint64_t container_hash_mir_type_entry(const void *lsv) {
  const mir_type_entry *l = lsv;
  return container_hash_ptr(l->type_ref);
}
static inline void container_delete_mir_type_entry(void *self) {
  mir_type_entry_free(self);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_mir_type_entry, mir_type_entry,
                              container_cmp_mir_type_entry, container_new_move,
                              container_delete_mir_type_entry,
                              container_hash_mir_type_entry);

// NAME_ENTRY
typedef struct mir_name_entry_struct {
  const symbol_entry *symbol_ref;
  void               *entry_ref;
} mir_name_entry;

static mir_name_entry *mir_name_entry_new(const symbol_entry *symbol_ref,
                                          void               *entry_ref) {
  mir_name_entry *self = MALLOC(mir_name_entry);
  self->symbol_ref     = symbol_ref;
  self->entry_ref      = entry_ref;
  return self;
}

static void mir_name_entry_free(mir_name_entry *self) {
  if (self) {
    free(self);
  }
}

static inline int container_cmp_mir_name_entry(const void *lsv,
                                               const void *rsv) {
  const mir_name_entry *l = lsv;
  const mir_name_entry *r = rsv;
  return container_cmp_chars(l->symbol_ref->name, r->symbol_ref->name);
}
static inline uint64_t container_hash_mir_name_entry(const void *lsv) {
  const mir_name_entry *l = lsv;
  return container_hash_chars(l->symbol_ref->name);
}
static inline void container_delete_mir_name_entry(void *self) {
  mir_name_entry_free(self);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_mir_name_entry, mir_name_entry,
                              container_cmp_mir_name_entry, container_new_move,
                              container_delete_mir_name_entry,
                              container_hash_mir_name_entry);

// CTX
typedef struct mir_ctx_struct {
  hashset_mir_sym_entry  *map_sym_sub;
  size_t                  lit_cnt;
  hashset_type_ref       *registered_types;
  hashset_mir_type_entry *map_type_hir_class;
  hashset_mir_type_entry *map_type_mir_class;

  // handled by subroutine
  mir_subroutine        *sub_ref;
  size_t                 value_cnt;
  size_t                 bb_cnt;
  hashset_mir_sym_entry *map_sym_value;
  list_mir_scope_ref    *scope_stack;
  mir_bb                *bb_last_ref;
  mir_bb                *bb_ref;

  mir              *mir;
  const type_table *type_table;
  const type_entry *type_any;
  list_exception   *exceptions;
} mir_ctx;

static void mir_ctx_init(mir_ctx *ctx, mir *mir, const type_table *type_table,
                         list_exception *exceptions) {
  ctx->map_sym_sub        = hashset_mir_sym_entry_new();
  ctx->lit_cnt            = 0;
  ctx->registered_types   = hashset_type_ref_new();
  ctx->map_type_hir_class = hashset_mir_type_entry_new();
  ctx->map_type_mir_class = hashset_mir_type_entry_new();

  ctx->sub_ref       = NULL;
  ctx->value_cnt     = 0;
  ctx->bb_cnt        = 0;
  ctx->map_sym_value = NULL;
  ctx->scope_stack   = NULL;
  ctx->bb_last_ref   = NULL;
  ctx->bb_ref        = NULL;

  ctx->mir        = mir;
  ctx->type_table = type_table;
  ctx->type_any   = NULL;
  ctx->exceptions = exceptions;
}

static void mir_ctx_deinit(mir_ctx *ctx) {
  hashset_mir_sym_entry_free(ctx->map_sym_sub);
  ctx->lit_cnt = 0;
  hashset_type_ref_free(ctx->registered_types);
  hashset_mir_type_entry_free(ctx->map_type_hir_class);
  hashset_mir_type_entry_free(ctx->map_type_mir_class);

  ctx->sub_ref       = NULL;
  ctx->value_cnt     = 0;
  ctx->bb_cnt        = 0;
  ctx->map_sym_value = NULL;
  ctx->scope_stack   = NULL;
  ctx->bb_last_ref   = NULL;
  ctx->bb_ref        = NULL;

  ctx->mir        = NULL;
  ctx->type_table = NULL;
  ctx->type_any   = NULL;
  ctx->exceptions = NULL;
}

static void mir_ctx_sym_sub_emplace(mir_ctx            *ctx,
                                    const symbol_entry *symbol_ref,
                                    mir_subroutine     *sub) {
  hashset_mir_sym_entry_insert(ctx->map_sym_sub,
                               mir_sym_entry_new(symbol_ref, sub));
}

static mir_subroutine *mir_ctx_sym_sub_find(mir_ctx            *ctx,
                                            const symbol_entry *symbol_ref) {
  hashset_mir_sym_entry_it it = hashset_mir_sym_entry_find(
      ctx->map_sym_sub, &(mir_sym_entry){.symbol_ref = symbol_ref});
  if (!END(it)) {
    return GET(it)->entry_ref;
  }
  return NULL;
}

static hir_class *mir_ctx_type_hir_class_find(mir_ctx          *ctx,
                                              const type_entry *type_ref) {
  hashset_mir_type_entry_it it = hashset_mir_type_entry_find(
      ctx->map_type_hir_class, &(mir_type_entry){.type_ref = type_ref});
  if (!END(it)) {
    return GET(it)->entry_ref;
  }
  return NULL;
}

static void mir_ctx_type_hir_class_emplace(mir_ctx          *ctx,
                                           const type_entry *type_ref,
                                           const hir_class *class) {
  hashset_mir_type_entry_insert(
      ctx->map_type_hir_class,
      mir_type_entry_new(type_ref, (hir_class *)class));
}

static mir_class *mir_ctx_type_mir_class_find(mir_ctx          *ctx,
                                              const type_entry *type_ref) {
  hashset_mir_type_entry_it it = hashset_mir_type_entry_find(
      ctx->map_type_mir_class, &(mir_type_entry){.type_ref = type_ref});
  if (!END(it)) {
    return GET(it)->entry_ref;
  }
  return NULL;
}

static mir_class *mir_ctx_type_mir_class_erase(mir_ctx *ctx,
                                               const mir_class *class) {
  hashset_mir_type_entry_it it = hashset_mir_type_entry_find(
      ctx->map_type_mir_class, &(mir_type_entry){.type_ref = class->type_ref});
  if (!END(it)) {
    mir_class *class = GET(it)->entry_ref;
    hashset_mir_type_entry_erase(ctx->map_type_mir_class, it);
    return class;
  }
  return NULL;
}

static void mir_ctx_type_mir_class_emplace(mir_ctx          *ctx,
                                           const type_entry *type_ref,
                                           const mir_class *class) {
  hashset_mir_type_entry_insert(
      ctx->map_type_mir_class,
      mir_type_entry_new(type_ref, (mir_class *)class));
}

static void mir_ctx_setup(mir_ctx *ctx, const hir *hir) {
  for (list_type_entry_it it = list_type_entry_begin(ctx->type_table->entries);
       !END(it); NEXT(it)) {
    hashset_type_ref_insert(ctx->registered_types, GET(it)->type);
  }
  // used for lazy instantiation of classes
  for (list_hir_class_it it = list_hir_class_begin(hir->classes); !END(it);
       NEXT(it)) {
    hir_class *class = GET(it);
    mir_ctx_type_hir_class_emplace(ctx, class->type_ref, class);
  }

  // add any type to type table
  type_base *type_any =
      (typeof(type_any))type_primitive_new(TYPE_PRIMITIVE_ANY);

  hashset_type_ref_it it =
      hashset_type_ref_find(ctx->registered_types, type_any);
  if (END(it)) {
    error("type any is not present in type table");
  } else {
    ctx->type_any = GET(it)->type_entry_ref;
  }

  type_free(type_any);
}

static inline void mir_ctx_sym_value_create(mir_ctx *ctx) {
  ctx->map_sym_value = hashset_mir_sym_entry_new();
}

static inline void mir_ctx_sym_value_destroy(mir_ctx *ctx) {
  hashset_mir_sym_entry_free(ctx->map_sym_value);
}

static void mir_ctx_sym_value_emplace(mir_ctx            *ctx,
                                      const symbol_entry *symbol_ref,
                                      mir_value          *value) {
  hashset_mir_sym_entry_insert(ctx->map_sym_value,
                               mir_sym_entry_new(symbol_ref, value));
}

static mir_value *mir_ctx_sym_value_find(mir_ctx            *ctx,
                                         const symbol_entry *symbol_ref) {
  hashset_mir_sym_entry_it it = hashset_mir_sym_entry_find(
      ctx->map_sym_value, &(mir_sym_entry){.symbol_ref = symbol_ref});
  if (!END(it)) {
    return GET(it)->entry_ref;
  }
  return NULL;
}

static mir_bb *mir_ctx_sub_emplace_back_bb(mir_ctx *ctx) {
  mir_bb *bb = mir_bb_new(ctx->bb_cnt++, list_mir_stmt_new(), NULL, NULL, NULL,
                          list_hir_expr_ref_new());
  list_mir_bb_push_back(ctx->sub_ref->defined.bbs, bb);
  return bb;
}

// EXPR_R
typedef struct mir_expr_o_struct {
  int lvalue; // passed to indicate that expr requires rvalue
} mir_expr_o;

typedef struct mir_expr_r_struct {
  mir_value *ret;
  mir_value *obj;

  int ref; // variable returns ref = 0, tmp can return ref = 1 (for assignment)
} mir_expr_r;

static inline mir_expr_o mir_expr_o_make(int lvalue) {
  return (mir_expr_o){.lvalue = lvalue};
}

static inline mir_expr_r mir_expr_r_make(mir_value *ret, mir_value *obj,
                                         int ref) {
  return (mir_expr_r){.ret = ret, .obj = obj, .ref = ref};
}

static inline mir_expr_r mir_expr_r_empty() {
  return (mir_expr_r){.ret = NULL, .ref = 0};
}

// constructs without subroutine body
// (to properly resolve references if used before declaration)
static mir_subroutine_spec
mir_declare_subroutine_spec(mir_ctx *ctx, hir_subroutine_spec spec) {
  UNUSED(ctx);

  switch (spec) {
    case HIR_SUBROUTINE_SPEC_EMPTY:
      return MIR_SUBROUTINE_SPEC_EMPTY;
    case HIR_SUBROUTINE_SPEC_EXTERN:
      return MIR_SUBROUTINE_SPEC_EXTERN;
  }

  error("unhandled mir subroutine spec %d", spec);
  return MIR_SUBROUTINE_SPEC_EMPTY;
}

static mir_subroutine *mir_declare_subroutine(mir_ctx              *ctx,
                                              const hir_subroutine *sub_hir) {
  mir_subroutine_spec spec = mir_declare_subroutine_spec(ctx, sub_hir->spec);

  if (!sub_hir->body) {
    return mir_subroutine_new_declared(sub_hir->id_ref, sub_hir->type_ref,
                                       spec);
  }

  switch (sub_hir->body->kind) {
    case HIR_SUBROUTINE_BODY_IMPORT: {
      const hir_lit *lib_lit   = sub_hir->body->body.import.lib;
      const hir_lit *entry_lit = sub_hir->body->body.import.entry;

      char *lib   = lib_lit ? strdup(lib_lit->value.v_str) : NULL;
      char *entry = entry_lit ? strdup(entry_lit->value.v_str) : NULL;

      return mir_subroutine_new_imported(sub_hir->id_ref, sub_hir->type_ref,
                                         spec, lib, entry);
    }
    case HIR_SUBROUTINE_BODY_BLOCK: {
      union {
        const type_base     *base;
        const type_callable *callable;
      } type;

      type.base = sub_hir->type_ref->type;

      if (type.base->kind != TYPE_CALLABLE) {
        mir_exception_add_error(ctx->exceptions, EXCEPTION_MIR_UNEXPECTED_TYPE,
                                sub_hir->id_ref->span,
                                "expected type %s, got %s %p",
                                type_enum_str(TYPE_CALLABLE),
                                type_enum_str(type.base->kind), sub_hir);
        return NULL;
      }

      ctx->value_cnt = 0;

      const type_base *type_ret = type.callable->ret_ref;

      mir_value *ret_value =
          mir_value_new(ctx->value_cnt++, NULL, type_ret->type_entry_ref);

      list_mir_value *params = list_mir_value_new();
      for (list_hir_param_it it = list_hir_param_begin(sub_hir->params);
           !END(it); NEXT(it)) {
        const hir_param *param_hir = GET(it);

        mir_value *param = mir_value_new(ctx->value_cnt++, param_hir->id_ref,
                                         param_hir->type_ref);
        list_mir_value_push_back(params, param);
      }

      return mir_subroutine_new_defined(sub_hir->id_ref, sub_hir->type_ref,
                                        spec, ret_value, params, NULL, NULL,
                                        NULL);
    }
  }
  error("unexpected subroutine body kind %d %p", sub_hir->body->kind, sub_hir);
  return NULL;
}

// EXPR
static mir_expr_r mir_define_expr(mir_ctx *ctx, const hir_expr_base *expr,
                                  mir_expr_o opts);

static list_mir_value_ref *mir_define_mir_value_args(mir_value *first_ref,
                                                     ...) {
  list_mir_value_ref *args = list_mir_value_ref_new();

  if (first_ref == NULL) {
    return args;
  }
  list_mir_value_ref_push_back(args, first_ref);

  va_list ap;
  va_start(ap, first_ref);

  while (1) {
    mir_value *arg = va_arg(ap, mir_value *);
    if (arg == NULL) {
      break;
    }
    list_mir_value_ref_push_back(args, arg);
  }

  va_end(ap);
  return args;
}

// returns reference
static mir_value *mir_define_mir_value_tmp(mir_ctx          *ctx,
                                           const type_entry *type_ref) {
  mir_value *res = mir_value_new(ctx->value_cnt++, NULL, type_ref);
  list_mir_value_push_back(ctx->sub_ref->defined.tmps, res);
  return res;
}

// returns reference
static mir_stmt *mir_define_mir_stmt(mir_ctx *ctx, mir_stmt *stmt) {
  list_mir_stmt_push_back(ctx->bb_ref->stmts, stmt);
  return stmt;
}

// returns reference
// type is not resolved
static mir_lit *mir_define_mir_lit(mir_ctx *ctx, const hir_expr_lit *expr) {
  const type_entry *entry_ref = expr->lit->type_ref;

  // validate
  int ok = 1;

  switch (entry_ref->type->kind) {
    case TYPE_PRIMITIVE:
      break;
    case TYPE_ARRAY:
    case TYPE_CALLABLE:
    case TYPE_CLASS_T:
    case TYPE_TYPENAME:
    case TYPE_MONO:
    default: {
      char *type_s = type_str(entry_ref->type);
      error("unsupported lit type %s %p", type_s, expr);
      ok = 0;
    } break;
  }

  if (!ok) {
    return NULL;
  }

  // build
  mir_lit_value value;
  hir_lit_u     hir_value = expr->lit->value;

  const type_primitive *type = (typeof(type))entry_ref->type;
  switch (type->type) {
    case TYPE_PRIMITIVE_BOOL:
      value.v_bool = hir_value.v_bool;
      break;
    case TYPE_PRIMITIVE_BYTE:
      value.v_byte = (uint8_t)hir_value.v_ulong;
      break;
    case TYPE_PRIMITIVE_INT:
      value.v_int = (int32_t)hir_value.v_long;
      break;
    case TYPE_PRIMITIVE_UINT:
      value.v_uint = (uint32_t)hir_value.v_ulong;
      break;
    case TYPE_PRIMITIVE_LONG:
      value.v_long = (int64_t)hir_value.v_long;
      break;
    case TYPE_PRIMITIVE_ULONG:
      value.v_ulong = (uint64_t)hir_value.v_ulong;
      break;
    case TYPE_PRIMITIVE_CHAR:
      value.v_char = (uint8_t)hir_value.v_char[0];
      break;
    case TYPE_PRIMITIVE_STRING:
      value.v_str = (uint8_t *)strdup(hir_value.v_str);
      break;
    case TYPE_PRIMITIVE_VOID:
      value.v_bool = 0;
      break;
    default:
      error("type %d is unresolved", type->type);
      value.v_bool = 0;
  }

  mir_lit *lit = mir_lit_new(ctx->lit_cnt++, entry_ref, value);
  list_mir_lit_push_back(ctx->mir->literals, lit);
  return lit;
}

// dereferences result if required
static mir_value *mir_define_expr_deref(mir_ctx *ctx, const mir_expr_r *mir_r,
                                        const span *span) {
  mir_value *value;

  mir_value *rvalue     = mir_define_mir_value_tmp(ctx, ctx->type_any);
  mir_stmt  *stmt_deref = mir_define_mir_stmt(
      ctx, mir_stmt_new_op(MIR_STMT_OP_DEREF, rvalue,
                            mir_define_mir_value_args(mir_r->ret, NULL)));

  value = rvalue;

  mir_stmt_debug_init_span(stmt_deref, span);

  return value;
}

static mir_expr_r mir_define_expr_unary_generic(mir_ctx              *ctx,
                                                const hir_expr_unary *expr,
                                                mir_stmt_op_enum      op) {
  mir_value *first = mir_define_expr(ctx, expr->first, mir_expr_o_make(0)).ret;
  mir_value *ret   = mir_define_mir_value_tmp(ctx, ctx->type_any);
  mir_stmt  *stmt  = mir_define_mir_stmt(
      ctx, mir_stmt_new_op(op, ret, mir_define_mir_value_args(first, NULL)));

  mir_stmt_debug_init_span(stmt, expr->base.base.span);

  return mir_expr_r_make(ret, NULL, 0);
}

static mir_expr_r mir_define_expr_unary(mir_ctx              *ctx,
                                        const hir_expr_unary *expr) {
  if (!expr) {
    return mir_expr_r_empty();
  }

  // validate
  int ok = 1;

  switch (expr->op) {
    case HIR_EXPR_UNARY_PLUS:
    case HIR_EXPR_UNARY_MINUS:
    case HIR_EXPR_UNARY_LOGICAL_NOT:
    case HIR_EXPR_UNARY_BITWISE_NOT:
      if (!expr->first) {
        ok = 0;
      }
      break;

    case HIR_EXPR_UNARY_INC:
    case HIR_EXPR_UNARY_DEC: {
      if (!expr->first) {
        ok = 0;
        break;
      }
      if (expr->first->kind == HIR_EXPR_IDENTIFIER) {
      } else if (expr->first->kind == HIR_EXPR_INDEX) {
      } else if (expr->first->kind == HIR_EXPR_BINARY &&
                 ((hir_expr_binary *)expr->first)->op ==
                     HIR_EXPR_BINARY_MEMBER) {
      } else {
        ok = 0;
        mir_exception_add_error(ctx->exceptions,
                                EXCEPTION_MIR_UNEXPECTED_RVALUE,
                                expr->base.base.span,
                                "expected identifier, index or member access");
      }
      break;
    }

    default:
      ok = 0;
      break;
  }

  if (!ok) {
    // debug("hir expr %p is invalid", expr);
    return mir_expr_r_empty();
  }

  // emit
  switch (expr->op) {
    case HIR_EXPR_UNARY_PLUS: {
      return mir_define_expr_unary_generic(ctx, expr, MIR_STMT_OP_UNARY_PLUS);
    }
    case HIR_EXPR_UNARY_MINUS: {
      return mir_define_expr_unary_generic(ctx, expr, MIR_STMT_OP_UNARY_MINUS);
    }
    case HIR_EXPR_UNARY_LOGICAL_NOT: {
      return mir_define_expr_unary_generic(ctx, expr,
                                           MIR_STMT_OP_UNARY_LOGICAL_NOT);
    }
    case HIR_EXPR_UNARY_BITWISE_NOT: {
      return mir_define_expr_unary_generic(ctx, expr,
                                           MIR_STMT_OP_UNARY_BITWISE_NOT);
    }
    case HIR_EXPR_UNARY_INC: {
      mir_expr_r first_r =
          mir_define_expr(ctx, expr->first, mir_expr_o_make(1));

      mir_value *first_deref;
      if (first_r.ref) {
        first_deref =
            mir_define_expr_deref(ctx, &first_r, expr->base.base.span);
      } else {
        first_deref = first_r.ret;
      }

      mir_value *ret = mir_define_mir_value_tmp(ctx, ctx->type_any);

      mir_stmt *stmt_inc = mir_define_mir_stmt(
          ctx, mir_stmt_new_op(MIR_STMT_OP_UNARY_INC, ret,
                               mir_define_mir_value_args(first_deref, NULL)));

      // save tmp result to actual value (preserve reference)
      mir_stmt *stmt_ass = mir_define_mir_stmt(
          ctx, mir_stmt_new_assign(MIR_STMT_ASSIGN_VALUE, first_r.ret, ret));

      mir_stmt_debug_init_span(stmt_inc, expr->base.base.span);
      mir_stmt_debug_init_span(stmt_ass, expr->base.base.span);

      return mir_expr_r_make(ret, NULL, 0);
    }
    case HIR_EXPR_UNARY_DEC: {
      mir_expr_r first_r =
          mir_define_expr(ctx, expr->first, mir_expr_o_make(1));

      mir_value *first_deref;
      if (first_r.ref) {
        first_deref =
            mir_define_expr_deref(ctx, &first_r, expr->base.base.span);
      } else {
        first_deref = first_r.ret;
      }

      mir_value *ret = mir_define_mir_value_tmp(ctx, ctx->type_any);

      mir_stmt *stmt_inc = mir_define_mir_stmt(
          ctx, mir_stmt_new_op(MIR_STMT_OP_UNARY_DEC, ret,
                               mir_define_mir_value_args(first_deref, NULL)));

      // save tmp result to actual value (preserve reference)
      mir_stmt *stmt_ass = mir_define_mir_stmt(
          ctx, mir_stmt_new_assign(MIR_STMT_ASSIGN_VALUE, first_r.ret, ret));

      mir_stmt_debug_init_span(stmt_inc, expr->base.base.span);
      mir_stmt_debug_init_span(stmt_ass, expr->base.base.span);

      return mir_expr_r_make(ret, NULL, 0);
    }
  }

  error("unhandled hir unary expr %s(%d) %p", hir_expr_unary_enum_str(expr->op),
        expr->op, expr);
  return mir_expr_r_empty();
}

static mir_expr_r mir_define_expr_binary_generic(mir_ctx               *ctx,
                                                 const hir_expr_binary *expr,
                                                 mir_stmt_op_enum       op) {
  mir_value *first = mir_define_expr(ctx, expr->first, mir_expr_o_make(0)).ret;
  mir_value *second =
      mir_define_expr(ctx, expr->second, mir_expr_o_make(0)).ret;
  mir_value *ret  = mir_define_mir_value_tmp(ctx, ctx->type_any);
  mir_stmt  *stmt = mir_define_mir_stmt(
      ctx,
      mir_stmt_new_op(op, ret, mir_define_mir_value_args(first, second, NULL)));

  mir_stmt_debug_init_span(stmt, expr->base.base.span);

  return mir_expr_r_make(ret, NULL, 0);
}

static mir_expr_r mir_define_expr_binary(mir_ctx               *ctx,
                                         const hir_expr_binary *expr,
                                         mir_expr_o             opts) {
  int ok = 1;

  if (!expr) {
    ok = 0;
  }

  switch (expr->op) {
    case HIR_EXPR_BINARY_MEMBER: {
      const type_entry *type_ref;

      if (expr->second->kind == HIR_EXPR_LITERAL &&
          (type_ref = ((hir_expr_lit *)expr->second)->lit->type_ref) &&
          type_ref->type->kind == TYPE_PRIMITIVE &&
          ((type_primitive *)type_ref->type)->type == TYPE_PRIMITIVE_STRING) {
      } else {
        mir_exception_add_error(
            ctx->exceptions, EXCEPTION_MIR_UNEXPECTED_MEMBER,
            expr->base.base.span,
            "member access only allowed with literal string");
        ok = 0;
      }

      break;
    }

    case HIR_EXPR_BINARY_ASSIGN:
      if (expr->first->kind == HIR_EXPR_BINARY &&
          ((hir_expr_binary *)expr->first)->op == HIR_EXPR_BINARY_MEMBER) {
      } else if (expr->first->kind == HIR_EXPR_IDENTIFIER) {
      } else if (expr->first->kind == HIR_EXPR_INDEX) {
      } else {
        mir_exception_add_error(
            ctx->exceptions, EXCEPTION_MIR_UNEXPECTED_MEMBER,
            expr->base.base.span,
            "assignment allowed only with members, index and vars");
        ok = 0;
      }
      break;

    case HIR_EXPR_BINARY_LOGICAL_OR:
    case HIR_EXPR_BINARY_LOGICAL_AND:
    case HIR_EXPR_BINARY_BITWISE_OR:
    case HIR_EXPR_BINARY_BITWISE_XOR:
    case HIR_EXPR_BINARY_BITWISE_AND:
    case HIR_EXPR_BINARY_EQUALS:
    case HIR_EXPR_BINARY_NOT_EQUALS:
    case HIR_EXPR_BINARY_LESS:
    case HIR_EXPR_BINARY_LESS_EQUALS:
    case HIR_EXPR_BINARY_GREATER:
    case HIR_EXPR_BINARY_GREATER_EQUALS:
    case HIR_EXPR_BINARY_BITWISE_SHIFT_LEFT:
    case HIR_EXPR_BINARY_BITWISE_SHIFT_RIGHT:
    case HIR_EXPR_BINARY_ADD:
    case HIR_EXPR_BINARY_SUB:
    case HIR_EXPR_BINARY_MUL:
    case HIR_EXPR_BINARY_DIV:
    case HIR_EXPR_BINARY_REM:
      break;
  }

  if (!ok) {
    return mir_expr_r_empty();
  }

  // build
  switch (expr->op) {
    case HIR_EXPR_BINARY_ASSIGN: {
      mir_value *from =
          mir_define_expr(ctx, expr->second, mir_expr_o_make(0)).ret;

      mir_expr_r to_r = mir_define_expr(ctx, expr->first, mir_expr_o_make(1));

      mir_stmt *stmt = mir_define_mir_stmt(
          ctx, mir_stmt_new_assign(MIR_STMT_ASSIGN_VALUE, to_r.ret, from));

      mir_value *to_deref;
      if (!opts.lvalue && to_r.ref) {
        to_deref = mir_define_expr_deref(ctx, &to_r, expr->base.base.span);
      } else {
        to_deref = to_r.ret;
      }

      mir_stmt_debug_init_span(stmt, expr->base.base.span);

      return mir_expr_r_make(to_deref, NULL, 0);
    }
    case HIR_EXPR_BINARY_LOGICAL_OR: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_LOGICAL_OR);
    }
    case HIR_EXPR_BINARY_LOGICAL_AND: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_LOGICAL_AND);
    }
    case HIR_EXPR_BINARY_BITWISE_OR: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_BITWISE_OR);
    }
    case HIR_EXPR_BINARY_BITWISE_XOR: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_BITWISE_XOR);
    }
    case HIR_EXPR_BINARY_BITWISE_AND: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_BITWISE_AND);
    }
    case HIR_EXPR_BINARY_EQUALS: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_EQUALS);
    }
    case HIR_EXPR_BINARY_NOT_EQUALS: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_NOT_EQUALS);
    }
    case HIR_EXPR_BINARY_LESS: {
      return mir_define_expr_binary_generic(ctx, expr, MIR_STMT_OP_BINARY_LESS);
    }
    case HIR_EXPR_BINARY_LESS_EQUALS: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_LESS_EQUALS);
    }
    case HIR_EXPR_BINARY_GREATER: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_GREATER);
    }
    case HIR_EXPR_BINARY_GREATER_EQUALS: {
      return mir_define_expr_binary_generic(ctx, expr,
                                            MIR_STMT_OP_BINARY_GREATER_EQUALS);
    }
    case HIR_EXPR_BINARY_BITWISE_SHIFT_LEFT: {
      return mir_define_expr_binary_generic(
          ctx, expr, MIR_STMT_OP_BINARY_BITWISE_SHIFT_LEFT);
    }
    case HIR_EXPR_BINARY_BITWISE_SHIFT_RIGHT: {
      return mir_define_expr_binary_generic(
          ctx, expr, MIR_STMT_OP_BINARY_BITWISE_SHIFT_RIGHT);
    }
    case HIR_EXPR_BINARY_ADD: {
      return mir_define_expr_binary_generic(ctx, expr, MIR_STMT_OP_BINARY_ADD);
    }
    case HIR_EXPR_BINARY_SUB: {
      return mir_define_expr_binary_generic(ctx, expr, MIR_STMT_OP_BINARY_SUB);
    }
    case HIR_EXPR_BINARY_MUL: {
      return mir_define_expr_binary_generic(ctx, expr, MIR_STMT_OP_BINARY_MUL);
    }
    case HIR_EXPR_BINARY_DIV: {
      return mir_define_expr_binary_generic(ctx, expr, MIR_STMT_OP_BINARY_DIV);
    }
    case HIR_EXPR_BINARY_REM: {
      return mir_define_expr_binary_generic(ctx, expr, MIR_STMT_OP_BINARY_REM);
    }
    case HIR_EXPR_BINARY_MEMBER: {
      mir_value *obj =
          mir_define_expr(ctx, expr->first, mir_expr_o_make(0)).ret;
      mir_lit *member = mir_define_mir_lit(ctx, (hir_expr_lit *)expr->second);

      mir_value *ret = mir_define_mir_value_tmp(ctx, ctx->type_any);

      mir_stmt *stmt;
      if (opts.lvalue) {
        stmt =
            mir_define_mir_stmt(ctx, mir_stmt_new_member_ref(ret, obj, member));
      } else {
        stmt = mir_define_mir_stmt(ctx, mir_stmt_new_member(ret, obj, member));
      }

      mir_stmt_debug_init_span(stmt, expr->base.base.span);

      return mir_expr_r_make(ret, obj, opts.lvalue);
    }
  }

  error("unhandled hir binary expr %s(%d) %p",
        hir_expr_binary_enum_str(expr->op), expr->op, expr);
  return mir_expr_r_empty();
}

static mir_expr_r mir_define_expr_lit(mir_ctx *ctx, const hir_expr_lit *expr) {
  mir_value *ret  = mir_define_mir_value_tmp(ctx, expr->lit->type_ref);
  mir_lit   *lit  = mir_define_mir_lit(ctx, expr);
  mir_stmt  *stmt = mir_define_mir_stmt(
      ctx, mir_stmt_new_assign(MIR_STMT_ASSIGN_LIT, ret, lit));

  mir_stmt_debug_init_span(stmt, expr->base.base.span);

  return mir_expr_r_make(ret, NULL, 0);
}

// if propagated here than need to return value
static mir_expr_r mir_define_expr_id(mir_ctx *ctx, const hir_expr_id *expr) {

  // check if in variables
  mir_value *value = mir_ctx_sym_value_find(ctx, expr->id_ref);
  if (value) {
    return mir_expr_r_make(value, NULL, 0);
  }

  // otherwise create tmp variable with subroutine
  mir_subroutine *subroutine = mir_ctx_sym_sub_find(ctx, expr->id_ref);
  if (subroutine) {
    mir_value *tmp  = mir_define_mir_value_tmp(ctx, ctx->type_any);
    mir_stmt  *stmt = mir_define_mir_stmt(
        ctx, mir_stmt_new_assign(MIR_STMT_ASSIGN_SUB, tmp, subroutine));

    mir_stmt_debug_init_span(stmt, expr->base.base.span);

    return mir_expr_r_make(tmp, NULL, 0);
  }

  mir_exception_add_error(
      ctx->exceptions, EXCEPTION_MIR_UNEXPECTED_IDENTIFIER, expr->id_ref->span,
      "no variable or subroutine %s found", expr->id_ref->name);

  return mir_expr_r_empty();
}

static mir_expr_r mir_define_expr_call(mir_ctx             *ctx,
                                       const hir_expr_call *expr) {

  list_mir_value_ref *args = list_mir_value_ref_new();
  for (list_hir_expr_it it = list_hir_expr_begin(expr->args); !END(it);
       NEXT(it)) {
    const hir_expr_base *arg = GET(it);
    list_mir_value_ref_push_back(
        args, mir_define_expr(ctx, arg, mir_expr_o_make(0)).ret);
  }

  switch (expr->callee->kind) {
    case HIR_EXPR_IDENTIFIER: {
      // if in subroutines then direct, else indirect (ex. call from variable)
      const hir_expr_id *callee = (typeof(callee))expr->callee;

      mir_subroutine *subroutine = mir_ctx_sym_sub_find(ctx, callee->id_ref);
      if (subroutine) {
        mir_value *ret = mir_define_mir_value_tmp(ctx, ctx->type_any);
        mir_stmt  *stmt =
            mir_define_mir_stmt(ctx, mir_stmt_new_call(ret, subroutine, args));

        mir_stmt_debug_init_span(stmt, expr->base.base.span);

        return mir_expr_r_make(ret, NULL, 0);
      }

      mir_value *indirect = mir_define_expr_id(ctx, callee).ret;
      if (indirect) {
        list_mir_value_ref_push_front(args, indirect);
        mir_value *ret  = mir_define_mir_value_tmp(ctx, ctx->type_any);
        mir_stmt  *stmt = mir_define_mir_stmt(
            ctx, mir_stmt_new_op(MIR_STMT_OP_CALL, ret, args));

        mir_stmt_debug_init_span(stmt, expr->base.base.span);

        return mir_expr_r_make(ret, NULL, 0);
      }

      error("shouldn't propagate here, identifier must be resoled");
      return mir_expr_r_empty();
    }

    case HIR_EXPR_UNARY:
    case HIR_EXPR_BINARY:
    case HIR_EXPR_LITERAL:
    case HIR_EXPR_CALL:
    case HIR_EXPR_INDEX:
    case HIR_EXPR_BUILTIN: {
      mir_expr_r indirect_r =
          mir_define_expr(ctx, expr->callee, mir_expr_o_make(0));

      // pass object if method call
      if (indirect_r.obj) {
        list_mir_value_ref_push_front(args, indirect_r.obj);
      }

      list_mir_value_ref_push_front(args, indirect_r.ret);

      mir_value *ret  = mir_define_mir_value_tmp(ctx, ctx->type_any);
      mir_stmt  *stmt = mir_define_mir_stmt(
          ctx, mir_stmt_new_op(MIR_STMT_OP_CALL, ret, args));

      mir_stmt_debug_init_span(stmt, expr->base.base.span);

      return mir_expr_r_make(ret, NULL, 0);
    }
  }

  list_mir_value_ref_free(args);
  error("unexpected callee kind %d %p", expr->callee->kind, expr);
  return mir_expr_r_empty();
}

static mir_expr_r mir_define_expr_index(mir_ctx              *ctx,
                                        const hir_expr_index *expr,
                                        mir_expr_o            opts) {
  list_mir_value_ref *args = list_mir_value_ref_new();
  for (list_hir_expr_it it = list_hir_expr_begin(expr->args); !END(it);
       NEXT(it)) {
    const hir_expr_base *arg = GET(it);
    list_mir_value_ref_push_back(
        args, mir_define_expr(ctx, arg, mir_expr_o_make(0)).ret);
  }

  mir_value *indirect = mir_define_expr(ctx, expr->indexed, opts).ret;

  if (indirect) {
    list_mir_value_ref_push_front(args, indirect);
    mir_value *ret = mir_define_mir_value_tmp(ctx, ctx->type_any);

    mir_stmt *stmt;
    if (opts.lvalue) {
      stmt = mir_define_mir_stmt(
          ctx, mir_stmt_new_op(MIR_STMT_OP_INDEX_REF, ret, args));
    } else {
      stmt = mir_define_mir_stmt(ctx,
                                 mir_stmt_new_op(MIR_STMT_OP_INDEX, ret, args));
    }

    mir_stmt_debug_init_span(stmt, expr->base.base.span);

    return mir_expr_r_make(ret, NULL, opts.lvalue);
  }

  list_mir_value_ref_free(args);
  return mir_expr_r_empty();
}

static mir_expr_r mir_define_expr_builtin_generic(mir_ctx                *ctx,
                                                  const hir_expr_builtin *expr,
                                                  mir_stmt_builtin_enum kind) {
  list_mir_value_ref *args = list_mir_value_ref_new();
  for (list_hir_expr_it it = list_hir_expr_begin(expr->args); !END(it);
       NEXT(it)) {
    const hir_expr_base *arg = GET(it);
    list_mir_value_ref_push_back(
        args, mir_define_expr(ctx, arg, mir_expr_o_make(0)).ret);
  }

  mir_value *ret  = mir_define_mir_value_tmp(ctx, ctx->type_any);
  mir_stmt  *stmt = mir_define_mir_stmt(
      ctx, mir_stmt_new_builtin(kind, ret, expr->base.type_ref, args));

  mir_stmt_debug_init_span(stmt, expr->base.base.span);

  return mir_expr_r_make(ret, NULL, 0);
}

static mir_expr_r mir_define_expr_builtin(mir_ctx                *ctx,
                                          const hir_expr_builtin *expr) {
  if (!expr) {
    return mir_expr_r_empty();
  }

  switch (expr->kind) {
    case HIR_EXPR_BUILTIN_CAST: {
      return mir_define_expr_builtin_generic(ctx, expr, MIR_STMT_BUILTIN_CAST);
    }
    case HIR_EXPR_BUILTIN_MAKE: {
      return mir_define_expr_builtin_generic(ctx, expr, MIR_STMT_BUILTIN_MAKE);
    }
    case HIR_EXPR_BUILTIN_PRINT: {
      return mir_define_expr_builtin_generic(ctx, expr, MIR_STMT_BUILTIN_PRINT);
    }
    case HIR_EXPR_BUILTIN_TYPE:
      return mir_define_expr_builtin_generic(ctx, expr, MIR_STMT_BUILTIN_TYPE);
  }

  error("unexpected builtin expr kind");
  return mir_expr_r_empty();
}

static mir_expr_r mir_define_expr(mir_ctx *ctx, const hir_expr_base *expr,
                                  mir_expr_o opts) {
  if (!expr) {
    return mir_expr_r_empty();
  }

  switch (expr->kind) {
    case HIR_EXPR_UNARY:
      return mir_define_expr_unary(ctx, (hir_expr_unary *)expr);
    case HIR_EXPR_BINARY:
      return mir_define_expr_binary(ctx, (hir_expr_binary *)expr, opts);
    case HIR_EXPR_LITERAL:
      return mir_define_expr_lit(ctx, (hir_expr_lit *)expr);
    case HIR_EXPR_IDENTIFIER:
      return mir_define_expr_id(ctx, (hir_expr_id *)expr);
    case HIR_EXPR_CALL:
      return mir_define_expr_call(ctx, (hir_expr_call *)expr);
    case HIR_EXPR_INDEX:
      return mir_define_expr_index(ctx, (hir_expr_index *)expr, opts);
    case HIR_EXPR_BUILTIN:
      return mir_define_expr_builtin(ctx, (hir_expr_builtin *)expr);
  }

  error("unexpected expr kind %d %p", expr->kind, expr);
  return mir_expr_r_empty();
}

static mir_value *mir_define_expr_tree(mir_ctx *ctx, mir_bb *bb,
                                       const hir_expr_base *expr) {
  ctx->bb_ref          = bb;
  mir_value *value_ref = mir_define_expr(ctx, expr, mir_expr_o_make(0)).ret;
  ctx->bb_ref          = NULL;
  return value_ref;
}

// STMTS
static mir_bb_seq mir_define_stmt(mir_ctx *ctx, const hir_stmt_base *stmt);

static inline mir_bb_seq mir_define_bb_seq_empty() {
  return (mir_bb_seq){.first = NULL, .last = NULL};
}

static mir_bb_seq mir_define_stmt_if(mir_ctx *ctx, const hir_stmt_if *stmt) {
  mir_bb_seq root = {
      .first = mir_ctx_sub_emplace_back_bb(ctx),
      .last  = mir_ctx_sub_emplace_back_bb(ctx),
  };

  root.first->jmp.cond_ref = mir_define_expr_tree(ctx, root.first, stmt->cond);
  list_hir_expr_ref_push_front(root.first->hir_exprs, stmt->cond);

  mir_bb_seq je = mir_define_stmt(ctx, stmt->je);
  if (mir_bb_get_cond(je.last) == MIR_BB_TERM) {
    je.last->jmp.next_ref = root.last;
  }
  root.first->jmp.je_ref = je.first;

  if (stmt->jz) {
    mir_bb_seq jz = mir_define_stmt(ctx, stmt->jz);
    if (mir_bb_get_cond(jz.last) == MIR_BB_TERM) {
      jz.last->jmp.next_ref = root.last;
    }
    root.first->jmp.jz_ref = jz.first;
  } else {
    root.first->jmp.jz_ref = root.last;
  }
  mir_bb_jmp_debug_init_span(root.first, stmt->cond->base.span);

  return root;
}

static mir_bb_seq mir_define_stmt_block(mir_ctx              *ctx,
                                        const hir_stmt_block *stmt) {
  mir_bb_seq root = {
      .first = mir_ctx_sub_emplace_back_bb(ctx),
      .last  = NULL,
  };

  mir_bb_seq prev = {
      .first = root.first,
      .last  = root.first,
  };

  for (list_hir_stmt_it it = list_hir_stmt_begin(stmt->stmts); !END(it);
       NEXT(it)) {
    const hir_stmt_base *hir_stmt = GET(it);
    mir_bb_seq           cur      = mir_define_stmt(ctx, hir_stmt);

    if (!cur.last) {
      warn("scope returned null");
      continue;
    }

    if (mir_bb_get_cond(prev.last) == MIR_BB_TERM) {
      prev.last->jmp.next_ref = cur.first;
    }
    prev = cur;
  }

  root.last = prev.last;
  return root;
}

static mir_bb_seq mir_define_stmt_while(mir_ctx              *ctx,
                                        const hir_stmt_while *stmt) {
  mir_bb_seq root = {
      .first = mir_ctx_sub_emplace_back_bb(ctx),
      .last  = mir_ctx_sub_emplace_back_bb(ctx),
  };

  list_mir_scope_ref_push_front(ctx->scope_stack, &root);

  root.first->jmp.cond_ref = mir_define_expr_tree(ctx, root.first, stmt->cond);
  list_hir_expr_ref_push_front(root.first->hir_exprs, stmt->cond);

  mir_bb_seq je = mir_define_stmt(ctx, stmt->stmt);
  if (mir_bb_get_cond(je.last) == MIR_BB_TERM) {
    je.last->jmp.next_ref = root.first;
  }

  root.first->jmp.je_ref = je.first;
  root.first->jmp.jz_ref = root.last;
  mir_bb_jmp_debug_init_span(root.first, stmt->cond->base.span);

  list_mir_scope_ref_pop_front(ctx->scope_stack);

  return root;
}

static mir_bb_seq mir_define_stmt_do(mir_ctx *ctx, const hir_stmt_do *stmt) {
  mir_bb_seq root = {
      .first = NULL,
      .last  = mir_ctx_sub_emplace_back_bb(ctx),
  };

  list_mir_scope_ref_push_front(ctx->scope_stack, &root);

  mir_bb_seq child_stmt = mir_define_stmt(ctx, stmt->stmt);
  root.first            = child_stmt.first;

  // handle condition in new block
  mir_bb *cond       = mir_ctx_sub_emplace_back_bb(ctx);
  cond->jmp.cond_ref = mir_define_expr_tree(ctx, cond, stmt->cond);
  list_hir_expr_ref_push_back(cond->hir_exprs, stmt->cond);

  if (stmt->positive) {
    cond->jmp.je_ref = root.first;
    cond->jmp.jz_ref = root.last;
  } else {
    cond->jmp.je_ref = root.last;
    cond->jmp.jz_ref = root.first;
  }
  mir_bb_jmp_debug_init_span(cond, stmt->cond->base.span);

  // link statement with condition if need to
  if (mir_bb_get_cond(child_stmt.last) == MIR_BB_TERM) {
    child_stmt.last->jmp.next_ref = cond;
  }

  list_mir_scope_ref_pop_front(ctx->scope_stack);

  return root;
}

static mir_bb_seq mir_define_stmt_break(mir_ctx              *ctx,
                                        const hir_stmt_break *stmt) {
  mir_bb_seq root = {.first = mir_ctx_sub_emplace_back_bb(ctx)};
  root.last       = root.first;

  if (list_mir_scope_ref_empty(ctx->scope_stack)) {
    mir_exception_add_error(ctx->exceptions, EXCEPTION_MIR_UNEXPECTED_BREAK,
                            stmt->base.base.span, "unable to break from here");
    return root;
  }

  mir_bb_seq *scope = list_mir_scope_ref_front(ctx->scope_stack);

  root.first->jmp.next_ref = scope->last;
  mir_bb_jmp_debug_init_span(root.first, stmt->base.base.span);

  return root;
}

static mir_bb_seq mir_define_stmt_expr(mir_ctx             *ctx,
                                       const hir_stmt_expr *stmt) {
  mir_bb_seq root = {.first = mir_ctx_sub_emplace_back_bb(ctx)};
  root.last       = root.first;

  mir_define_expr_tree(ctx, root.first, stmt->expr);
  list_hir_expr_ref_push_back(root.first->hir_exprs, stmt->expr);

  return root;
}

static mir_bb_seq mir_define_stmt_return(mir_ctx               *ctx,
                                         const hir_stmt_return *stmt) {
  mir_bb_seq root = {.first = mir_ctx_sub_emplace_back_bb(ctx)};
  root.last       = root.first;

  mir_value *value = mir_define_expr_tree(ctx, root.first, stmt->expr);
  list_hir_expr_ref_push_back(root.first->hir_exprs, stmt->expr);

  mir_stmt *stmt_ass = mir_stmt_new_assign(MIR_STMT_ASSIGN_VALUE,
                                           ctx->sub_ref->defined.ret, value);
  list_mir_stmt_push_back(root.first->stmts, stmt_ass);

  mir_stmt_debug_init_span(stmt_ass, stmt->base.base.span);

  root.last->jmp.next_ref = ctx->bb_last_ref;
  mir_bb_jmp_debug_init_span(root.last, stmt->base.base.span);

  return root;
}

static mir_bb_seq mir_define_stmt(mir_ctx *ctx, const hir_stmt_base *stmt) {
  if (!stmt) {
    return mir_define_bb_seq_empty();
  }

  switch (stmt->kind) {
    case HIR_STMT_IF:
      return mir_define_stmt_if(ctx, (hir_stmt_if *)stmt);
    case HIR_STMT_BLOCK:
      return mir_define_stmt_block(ctx, (hir_stmt_block *)stmt);
    case HIR_STMT_WHILE:
      return mir_define_stmt_while(ctx, (hir_stmt_while *)stmt);
    case HIR_STMT_DO:
      return mir_define_stmt_do(ctx, (hir_stmt_do *)stmt);
    case HIR_STMT_BREAK:
      return mir_define_stmt_break(ctx, (hir_stmt_break *)stmt);
    case HIR_STMT_EXPR:
      return mir_define_stmt_expr(ctx, (hir_stmt_expr *)stmt);
    case HIR_STMT_RETURN:
      return mir_define_stmt_return(ctx, (hir_stmt_return *)stmt);
  }

  error("unexpectes stmt kind %d %p", stmt->kind, stmt);
  return mir_define_bb_seq_empty();
}

static void mir_define_subroutine_defined(mir_ctx *ctx, mir_subroutine *sub_mir,
                                          const hir_subroutine *sub_hir) {
  const list_hir_var   *hir_vars = sub_hir->body->body.block.vars;
  const hir_stmt_block *hir_root = sub_hir->body->body.block.block;

  ctx->sub_ref   = sub_mir;
  ctx->value_cnt = 1 + list_mir_value_size(sub_mir->defined.params);
  ctx->bb_cnt    = 0;

  // add vars
  sub_mir->defined.vars = list_mir_value_new();

  for (list_hir_var_it it = list_hir_var_begin(hir_vars); !END(it); NEXT(it)) {
    const hir_var *var_hir = GET(it);

    mir_value *value =
        mir_value_new(ctx->value_cnt++, var_hir->id_ref, var_hir->type_ref);
    list_mir_value_push_back(sub_mir->defined.vars, value);
  }

  // add values to symbol scope
  mir_ctx_sym_value_create(ctx);

  for (list_mir_value_it it = list_mir_value_begin(sub_mir->defined.params);
       !END(it); NEXT(it)) {
    mir_value *param = GET(it);
    mir_ctx_sym_value_emplace(ctx, param->symbol_ref, param);
  }
  for (list_mir_value_it it = list_mir_value_begin(sub_mir->defined.vars);
       !END(it); NEXT(it)) {
    mir_value *param = GET(it);
    mir_ctx_sym_value_emplace(ctx, param->symbol_ref, param);
  }

  // handle body
  sub_mir->defined.tmps = list_mir_value_new();
  sub_mir->defined.bbs  = list_mir_bb_new();
  ctx->scope_stack      = list_mir_scope_ref_new();

  // add last block to handle returns
  mir_bb *bb_last  = mir_bb_new(ctx->bb_cnt++, list_mir_stmt_new(), NULL, NULL,
                                NULL, list_hir_expr_ref_new());
  ctx->bb_last_ref = bb_last;

  mir_bb_seq seq = mir_define_stmt_block(ctx, hir_root);

  // link what is done with last block and push back last block
  if (mir_bb_get_cond(seq.last) == MIR_BB_TERM) {
    seq.last->jmp.next_ref = ctx->bb_last_ref;
  }
  list_mir_bb_push_back(ctx->sub_ref->defined.bbs, bb_last);

  list_mir_scope_ref_free(ctx->scope_stack);
  mir_ctx_sym_value_destroy(ctx);
}

// if already instantiated (return it)
static mir_class *mir_define_class(mir_ctx          *ctx,
                                   const type_entry *class_entry) {
  if (!class_entry) {
    return NULL;
  }

  // already instantiated
  mir_class *class = mir_ctx_type_mir_class_find(ctx, class_entry);
  if (class) {
    return class;
  }

  const hir_class *class_hir = mir_ctx_type_hir_class_find(ctx, class_entry);

  if (!class_hir) {
    char *type_s = type_str(class_entry->type);
    error("hir class %s not found", type_s);
    if (type_s) {
      free(type_s);
    }
    return NULL;
  }

  class = mir_class_new(class_entry, list_mir_value_new(),
                        list_mir_subroutine_ref_new());

  // get parents
  type_base     *class_type  = class->type_ref->type;
  list_type_ref *parents_ref = NULL;
  switch (class_type->kind) {
    case TYPE_MONO: {
      type_mono *self = (typeof(self))class_type;

      if (self->type_ref->kind != TYPE_CLASS_T) {
        char *type_s = type_str(self->type_ref);
        error("unexpected mono ref %s(%d) %p", type_s, self->type_ref->kind,
              self);
        if (type_s) {
          free(type_s);
        }
        break;
      }

      parents_ref = ((type_class_t *)self->type_ref)->parents;
      break;
    }
    default: {
      char *type_s = type_str(class_type);
      error("unexpected type kind %s(%d) %p", type_s, class_type->kind,
            class_type);
      if (type_s) {
        free(type_s);
      }
    }
  }
  if (!parents_ref) {
    error("parent list is null, exiting");
    mir_class_free(class);
    return NULL;
  }

  hashset_symbol_entry *class_symbols = hashset_symbol_entry_new();
  // point to pointer of list of methods
  hashset_mir_name_entry *class_methods = hashset_mir_name_entry_new();
  size_t                  fields_cnt    = 0;

  // merge fields
  for (list_type_ref_it it = list_type_ref_begin(parents_ref); !END(it);
       NEXT(it)) {
    type_base *parent_type = GET(it);
    mir_class *parent      = mir_define_class(ctx, parent_type->type_entry_ref);

    if (!parent) {
      continue;
    }

    for (list_mir_value_it v_it = list_mir_value_begin(parent->fields);
         !END(v_it); NEXT(v_it)) {
      mir_value *field = GET(v_it);

      hashset_symbol_entry_it sym_it = hashset_symbol_entry_find(
          class_symbols, (symbol_entry *)field->symbol_ref);

      if (!END(sym_it)) {
        char               *class_s  = type_str(class_type);
        char               *parent_s = type_str(parent_type);
        const symbol_entry *symbol   = GET(sym_it);
        mir_exception_add_error(
            ctx->exceptions, EXCEPTION_MIR_SYMBOL_NAME_COLLISION, symbol->span,
            "%s parent %s var redefines symbol '%s' defined in another parent",
            class_s, parent_s, symbol->name);
        if (class_s)
          free(class_s);
        if (parent_s)
          free(parent_s);
        continue;
      }

      mir_value *field_new =
          mir_value_new(fields_cnt++, field->symbol_ref, field->type_ref);

      list_mir_value_push_back(class->fields, field_new);

      hashset_symbol_entry_insert(class_symbols,
                                  (symbol_entry *)field_new->symbol_ref);
    }
  }

  // instantiate its fields
  for (list_hir_var_it it = list_hir_var_begin(class_hir->fields); !END(it);
       NEXT(it)) {
    const hir_var *var_hir = GET(it);

    hashset_symbol_entry_it sym_it =
        hashset_symbol_entry_find(class_symbols, var_hir->id_ref);

    if (!END(sym_it)) {
      char               *class_s = type_str(class_type);
      const symbol_entry *symbol  = GET(sym_it);
      mir_exception_add_error(
          ctx->exceptions, EXCEPTION_MIR_SYMBOL_NAME_COLLISION, symbol->span,
          "%s var redefines symbol '%s' defined in another parent", class_s,
          symbol->name);
      if (class_s)
        free(class_s);
      continue;
    }

    mir_value *var =
        mir_value_new(fields_cnt++, var_hir->id_ref, var_hir->type_ref);

    list_mir_value_push_back(class->fields, var);
    hashset_symbol_entry_insert(class_symbols, (symbol_entry *)var->symbol_ref);
  }

  // merge methods
  for (list_type_ref_it it = list_type_ref_begin(parents_ref); !END(it);
       NEXT(it)) {
    type_base *parent_type = GET(it);
    mir_class *parent      = mir_define_class(ctx, parent_type->type_entry_ref);

    if (!parent) {
      continue;
    }

    for (list_mir_subroutine_ref_it s_it =
             list_mir_subroutine_ref_begin(parent->methods);
         !END(s_it); NEXT(s_it)) {
      mir_subroutine *method = GET(s_it);

      hashset_symbol_entry_it sym_it = hashset_symbol_entry_find(
          class_symbols, (symbol_entry *)method->symbol_ref);

      // add exception if this symbol is already defined by another parent
      if (!END(sym_it)) {
        char               *class_s  = type_str(class_type);
        char               *parent_s = type_str(parent_type);
        const symbol_entry *symbol   = GET(sym_it);
        mir_exception_add_error(
            ctx->exceptions, EXCEPTION_MIR_SYMBOL_NAME_COLLISION, symbol->span,
            "%s parent %s method redefines symbol '%s' defined in another "
            "parent",
            class_s, parent_s, symbol->name);
        if (class_s)
          free(class_s);
        if (parent_s)
          free(parent_s);
        continue;
      }

      hashset_symbol_entry_insert(class_symbols,
                                  (symbol_entry *)method->symbol_ref);
      hashset_mir_name_entry_insert(
          class_methods, mir_name_entry_new(method->symbol_ref, method));
    }
  }

  // instantiate its methods (override)
  for (list_hir_method_it it = list_hir_method_begin(class_hir->methods);
       !END(it); NEXT(it)) {
    const hir_method *method_hir    = GET(it);
    symbol_entry     *method_symbol = method_hir->subroutine->id_ref;

    hashset_symbol_entry_it sym_it =
        hashset_symbol_entry_find(class_symbols, method_symbol);

    hashset_mir_name_entry_it method_it = hashset_mir_name_entry_find(
        class_methods, &(mir_name_entry){.symbol_ref = method_symbol});

    // found as var from another parent
    if (!END(sym_it) && END(method_it)) {
      char               *class_s = type_str(class_type);
      const symbol_entry *symbol  = GET(sym_it);
      mir_exception_add_error(ctx->exceptions,
                              EXCEPTION_MIR_SYMBOL_NAME_COLLISION, symbol->span,
                              "%s method redefines symbol method '%s' defined "
                              "in another parent as var",
                              class_s, symbol->name);
      if (class_s)
        free(class_s);
      continue;
    }

    mir_subroutine *method;

    // declare+define need subroutine if not already defined
    if (!(method = mir_ctx_sym_sub_find(ctx, method_symbol))) {
      method = mir_declare_subroutine(ctx, method_hir->subroutine);
      if (!method) {
        error("method %s hasn't been declared", method_symbol->name);
        continue;
      }
      if (method->kind == MIR_SUBROUTINE_DEFINED) {
        mir_define_subroutine_defined(ctx, method, method_hir->subroutine);
      }
      list_mir_subroutine_push_back(ctx->mir->methods, method);
      mir_ctx_sym_sub_emplace(ctx, method_symbol, method);
    }

    mir_name_entry *method_entry =
        mir_name_entry_new(method->symbol_ref, method);

    if (!END(method_it)) {
      // override
      hashset_mir_name_entry_replace(class_methods, method_it, method_entry);
    } else {
      hashset_mir_name_entry_insert(class_methods, method_entry);
    }
  }

  // add resolved methods to class method list
  for (hashset_mir_name_entry_it it =
           hashset_mir_name_entry_begin(class_methods);
       !END(it); NEXT(it)) {
    list_mir_subroutine_ref_push_back(class->methods, GET(it)->entry_ref);
  }

  hashset_symbol_entry_free(class_symbols);
  hashset_mir_name_entry_free(class_methods);

  // add to index
  mir_ctx_type_mir_class_emplace(ctx, class_entry, class);

  return class;
}

static void mir_lower_subs(mir_ctx *ctx, const hir *hir) {
  for (list_hir_subroutine_it it = list_hir_subroutine_begin(hir->subroutines);
       !END(it); NEXT(it)) {
    const hir_subroutine *sub_hir = GET(it);

    mir_subroutine *sub = mir_declare_subroutine(ctx, sub_hir);
    if (!sub) {
      continue;
    }

    // add to index (to then parse bodies and resolve references)
    mir_ctx_sym_sub_emplace(ctx, sub->symbol_ref, sub);

    switch (sub->kind) {
      case MIR_SUBROUTINE_DEFINED:
        list_mir_subroutine_push_back(ctx->mir->defined_subs, sub);
        break;
      case MIR_SUBROUTINE_DECLARED:
        list_mir_subroutine_push_back(ctx->mir->declared_subs, sub);
        break;
      case MIR_SUBROUTINE_IMPORTED:
        list_mir_subroutine_push_back(ctx->mir->imported_subs, sub);
        break;
    }
  }

  for (list_hir_subroutine_it it = list_hir_subroutine_begin(hir->subroutines);
       !END(it); NEXT(it)) {
    const hir_subroutine *sub_hir = GET(it);

    mir_subroutine *sub_mir;
    if (!(sub_mir = mir_ctx_sym_sub_find(ctx, sub_hir->id_ref))) {
      error("subroutine %s not found %p", sub_hir->id_ref->name, sub_hir);
      continue;
    }

    if (sub_mir->kind == MIR_SUBROUTINE_DEFINED) {
      mir_define_subroutine_defined(ctx, sub_mir, sub_hir);
    }
  }
}

static void mir_lower_classes(mir_ctx *ctx, const hir *hir) {
  for (list_hir_class_it it = list_hir_class_begin(hir->classes); !END(it);
       NEXT(it)) {
    const hir_class *class_hir = GET(it);

    mir_class *class_mir = mir_define_class(ctx, class_hir->type_ref);
    if (!class_mir) {
      continue;
    }

    list_mir_class_push_back(ctx->mir->classes, class_mir);
  }

  // remove classes that don't need to be instantiated from index
  // (because they are stored as references and won't be removed automatically)
  for (list_mir_class_it it = list_mir_class_begin(ctx->mir->classes); !END(it);
       NEXT(it)) {
    mir_ctx_type_mir_class_erase(ctx, GET(it));
  }

  for (hashset_mir_type_entry_it it =
           hashset_mir_type_entry_begin(ctx->map_type_mir_class);
       !END(it); NEXT(it)) {
    mir_type_entry *entry = GET(it);
    mir_class *class      = entry->entry_ref;
    mir_class_free(class);
  }

  // WARN: map is freed, because iterator invalidates while erasing
  hashset_mir_type_entry_free(ctx->map_type_mir_class);
  ctx->map_type_mir_class = hashset_mir_type_entry_new();

  for (list_mir_class_it it = list_mir_class_begin(ctx->mir->classes); !END(it);
       NEXT(it)) {
    mir_class *class = GET(it);
    mir_ctx_type_mir_class_emplace(ctx, class->type_ref, class);
  }
}

mir_lower_hir_result mir_lower_hir(const hir        *hir,
                                   const type_table *type_table) {
  mir_lower_hir_result result = {
      .mir        = mir_new(),
      .exceptions = list_exception_new(),
  };

  mir_ctx ctx;
  mir_ctx_init(&ctx, result.mir, type_table, result.exceptions);
  mir_ctx_setup(&ctx, hir);

  mir_lower_subs(&ctx, hir);
  mir_lower_classes(&ctx, hir);

  mir_ctx_deinit(&ctx);

  return result;
}
