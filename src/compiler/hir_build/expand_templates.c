#include "expand_templates.h"

#include "compiler/hir_build/exception.h"
#include "compiler/type_table/str.h"
#include "util/hashset.h"
#include "util/log.h"
#include "util/macro.h"
#include <stdio.h>
#include <string.h>

// 1. erase classes with templates from hir list, add to the list for expansion
// 2. add templates to list of to be expanded
// 3. expand preorder, add types that aren't currently expanded
// 4. infinite recursion -> stack overflow (maybe handle different way later)

LIST_DECLARE_STATIC_INLINE(list_type_mono_ref, type_mono, container_cmp_false,
                           container_new_move, container_delete_false);

// TYPE_TEMPLATE ENTRY (for resolution)
typedef struct hir_type_tmpl_struct {
  const type_base *type_ref; // which type template to use (is not mono-full)
  const hir_base  *template_ref; // which hir class template to use
} hir_type_tmpl;

static hir_type_tmpl *hir_type_tmpl_new(const type_base *type_ref,
                                        const hir_base  *template_ref) {
  hir_type_tmpl *self = MALLOC(hir_type_tmpl);
  self->type_ref      = type_ref;
  self->template_ref  = template_ref;
  return self;
}

static void hir_type_tmpl_free(hir_type_tmpl *self) {
  if (self) {
    free(self);
  }
}

static inline void container_delete_hir_type_tmpl(void *self) {
  hir_type_tmpl_free(self);
}
static inline int container_cmp_hir_type_tmpl(const void *lsv,
                                              const void *rsv) {
  const hir_type_tmpl *l = lsv;
  const hir_type_tmpl *r = rsv;
  return container_cmp_ptr(l->type_ref, r->type_ref);
}
static inline uint64_t container_hash_hir_type_tmpl(const void *lsv) {
  const hir_type_tmpl *l = lsv;
  return container_hash_ptr(l->type_ref);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_hir_type_tmpl, hir_type_tmpl,
                              container_cmp_hir_type_tmpl, container_new_move,
                              container_delete_hir_type_tmpl,
                              container_hash_hir_type_tmpl);

// TYPE INDEX
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

// TYPE_MAPPINGS
typedef struct hir_type_map_struct {
  const type_base *from;
  const type_base *to;
} hir_type_map;

static hir_type_map *hir_type_map_new(const type_base *from,
                                      const type_base *to) {
  hir_type_map *self = MALLOC(hir_type_map);
  self->from         = from;
  self->to           = to;
  return self;
}

static void hir_type_map_free(hir_type_map *self) {
  if (self) {
    free(self);
  }
}

static inline void container_delete_hir_type_map(void *self) {
  hir_type_map_free(self);
}
static inline int container_cmp_hir_type_map(const void *lsv, const void *rsv) {
  const hir_type_map *l = lsv;
  const hir_type_map *r = rsv;
  return container_cmp_ptr(l->from, r->from);
}
static inline uint64_t container_hash_hir_type_map(const void *lsv) {
  const hir_type_map *l = lsv;
  return container_hash_ptr(l->from);
}
HASHSET_DECLARE_STATIC_INLINE(hashset_hir_type_map, hir_type_map,
                              container_cmp_hir_type_map, container_new_move,
                              container_delete_hir_type_map,
                              container_hash_hir_type_map);

// CONTEXT
typedef struct hir_ctx_struct {
  list_hir_class        *hir_tmpl_classes;  // stores actual class templates
  hashset_hir_type_tmpl *map_type_hir_tmpl; // matches types and their templates
  hashset_type_ref      *insted_types; // already instantiated/resolved types
  hashset_type_ref      *registered_types; // present in type table
  list_type_mono_ref    *instto_types;

  // no scopes as there is not constructions for recursive expansion
  hashset_hir_type_map *map_type_tmpl;

  type_table     *type_table;
  list_hir_class *hir_classes;
  list_exception *exceptions;
} hir_ctx;

static void hir_ctx_init(hir_ctx *ctx, type_table *type_table,
                         list_exception *exceptions,
                         list_hir_class *hir_classes) {
  ctx->hir_tmpl_classes  = list_hir_class_new();
  ctx->map_type_hir_tmpl = hashset_hir_type_tmpl_new();
  ctx->insted_types      = hashset_type_ref_new();
  ctx->registered_types  = hashset_type_ref_new();
  ctx->instto_types      = list_type_mono_ref_new();

  ctx->map_type_tmpl = NULL;

  ctx->type_table  = type_table;
  ctx->hir_classes = hir_classes;
  ctx->exceptions  = exceptions;
}

static void hir_ctx_deinit(hir_ctx *ctx) {
  list_hir_class_free(ctx->hir_tmpl_classes);
  hashset_hir_type_tmpl_free(ctx->map_type_hir_tmpl);
  hashset_type_ref_free(ctx->insted_types);
  hashset_type_ref_free(ctx->registered_types);
  list_type_mono_ref_free(ctx->instto_types);

  ctx->map_type_tmpl = NULL; // typenames in this context

  ctx->type_table  = NULL;
  ctx->hir_classes = NULL;
  ctx->exceptions  = NULL;
}

static type_base *hir_ctx_type_tmpl_find(hir_ctx *ctx, const type_base *type) {
  hashset_hir_type_map_it it = hashset_hir_type_map_find(
      ctx->map_type_tmpl, &(hir_type_map){.from = type});
  if (END(it)) {
    return NULL;
  }
  return (type_base *)GET(it)->to;
}

// add to registered types and add to type_table
static type_base *hir_ctx_type_register(hir_ctx *ctx, type_base *type,
                                        const span *span_ref) {

  hashset_type_ref_it it = hashset_type_ref_find(ctx->registered_types, type);

  if (END(it)) {
    hashset_type_ref_insert(ctx->registered_types, type);
    type_entry *entry =
        type_table_emplace(ctx->type_table, type, span_copy(span_ref));

    char *type_s = type_str(type);
    debug("registered %s", type_s);
    if (type_s)
      free(type_s);

    return entry->type;
  }

  type_free(type);
  return GET(it);
}

static void hir_ctx_debug(const hir_ctx *ctx, const char *s) {
  debug("hir_ctx: (%s)", s);

  debug("  hir_tmpl_classes -> size: %lu",
        list_hir_class_size(ctx->hir_tmpl_classes));
  debug("  map_type_hir_tmpl (index) -> size: %lu",
        ctx->map_type_hir_tmpl->hashset.size);
  debug("  insted_types (index) -> size: %lu", ctx->insted_types->hashset.size);
  debug("  registered_types (index) -> size: %lu",
        ctx->registered_types->hashset.size);
  debug("  instto_types -> size: %lu",
        list_type_mono_ref_size(ctx->instto_types));

  debug("  exceptions -> size: %lu", list_exception_size(ctx->exceptions));
}

// filters out templated classes from hir->classes
// rest stay in hir->class, templates are returned by this function
static list_hir_class *hir_filter_templated_hir_classes(hir *hir) {
  list_hir_class *classes_t = list_hir_class_new(); // to be expanded
                                                    //
  size_t hir_class_size = list_hir_class_size(hir->classes);

  for (size_t i = 0; i < hir_class_size; ++i) {
    hir_class *hir_class = list_hir_class_pop_front(hir->classes);

    if (!(hir_class->state &= HIR_STATE_BIND_TYPE)) {
      error("illegal state, expected class type %s type to be binded");
      continue;
    }

    type_base *class_type = hir_class->type_ref->type;

    type_mono_enum type_mono = type_fetch_mono(class_type);
    switch (type_mono) {
      case TYPE_MONO_NOT_FULL: {
        list_hir_class_push_back(classes_t, hir_class);
        break;
      }
      case TYPE_MONO_FULL: {
        list_hir_class_push_back(hir->classes, hir_class);
        break;
      }
      case TYPE_MONO_UNSET:
      default: {
        char *type_s = type_str(class_type);
        error("mono should be set for class with type %s %p", type_s,
              hir_class);
        if (type_s) {
          free(type_s);
        }
        break;
      }
    }
  }

  return classes_t;
}

// get class types that need to be instantiated
static list_type_mono_ref *
hir_get_mono_full_class_types(type_table *type_table) {
  list_type_mono_ref *classes_t = list_type_mono_ref_new();

  for (list_type_entry_it it = list_type_entry_begin(type_table->entries);
       !END(it); NEXT(it)) {
    const type_entry *entry = GET(it);
    type_base        *type  = entry->type;

    type_mono_enum mono = type_fetch_mono(type);
    switch (mono) {
      case TYPE_MONO_FULL: {
        if (type->kind == TYPE_MONO) {
          type_mono *mono = (type_mono *)type;
          type_enum  kind = mono->type_ref->kind;
          if (kind == TYPE_CLASS_T) {
            list_type_mono_ref_push_back(classes_t, mono);
          } else if (mono->type_ref->kind == TYPE_MONO) {
            error("unexpected mono as base for mono (%p -> %p)", mono,
                  mono->type_ref);
          }
        }
        break;
      }
      case TYPE_MONO_NOT_FULL: {
        break;
      }
      case TYPE_MONO_UNSET:
      default: {
        char *type_s = type_str(type);
        error("mono should be set for class with type %s %p", type_s, type);
        if (type_s)
          free(type_s);
        break;
      }
    }
  }
  return classes_t;
}

// adds errors if can't instantiate
static const hir_base *hir_get_hir_template(hir_ctx         *ctx,
                                            const type_mono *mono) {
  type_enum kind = mono->base.kind;

  if (kind != TYPE_MONO) {
    error("expecting mono types to fill class_inst_q, got %s(%d)",
          type_enum_str(kind), kind);
    return NULL;
  }

  hashset_hir_type_tmpl_it it = hashset_hir_type_tmpl_find(
      ctx->map_type_hir_tmpl, &(hir_type_tmpl){.type_ref = mono->type_ref});

  if (!END(it)) {
    return GET(it)->template_ref;
  }

  char *type_s = type_str((type_base *)mono);
  hir_exception_add_error(ctx->exceptions, EXCEPTION_HIR_TEMPLATE_EXPANSION,
                          mono->base.type_entry_ref->span,
                          "can't find template to instantiate %s", type_s);
  if (type_s)
    free(type_s);

  return NULL;
}

// add class templates that can be used for instantiation
static void hir_setup_ctx_hir_templates(hir_ctx        *ctx,
                                        list_hir_class *templated_classes) {
  while (!list_hir_class_empty(templated_classes)) {
    hir_class     *hir = list_hir_class_pop_front(templated_classes);
    hir_type_tmpl *type_tmpl =
        hir_type_tmpl_new(hir->type_ref->type, (hir_base *)hir);

    hashset_hir_type_tmpl_insert(ctx->map_type_hir_tmpl, type_tmpl);

    list_hir_class_push_back(ctx->hir_tmpl_classes, hir);
  }
  list_hir_class_free(templated_classes);
}

// register types
static void hir_setup_ctx(hir_ctx *ctx) {
  for (list_type_entry_it it = list_type_entry_begin(ctx->type_table->entries);
       !END(it); NEXT(it)) {
    hashset_type_ref_insert(ctx->registered_types, GET(it)->type);
  }
}

static void hir_setup_ctx_instto_types(hir_ctx            *ctx,
                                       list_type_mono_ref *instto_types) {
  while (!list_type_mono_ref_empty(instto_types)) {
    type_mono *mono = list_type_mono_ref_pop_front(instto_types);
    list_type_mono_ref_push_back(ctx->instto_types, mono);
  }
  list_type_mono_ref_free(instto_types);
}

// unwrap recursively, because without constructions like
// class B<T> : A<array [] of T> doesn't work
static type_base *hir_expand_type(hir_ctx *ctx, type_base *type) {
  if (!type) {
    return NULL;
  }

  // char *type_s = type_str(type);
  // info("expanding %s", type_s);
  // if (type_s)
  //   free(type_s);

  int type_changed = 0;

  switch (type_fetch_mono(type)) {
    default:
    case TYPE_MONO_UNSET: {
      char *type_s = type_str(type);
      error("type %s mono is unset", type_s);
      if (type_s) {
        free(type_s);
      }
      return NULL;
    }

    case TYPE_MONO_FULL: {
      return type;
    }

    case TYPE_MONO_NOT_FULL:
      // try expand lazily recursively
      switch (type->kind) {
        case TYPE_PRIMITIVE:
          return type;
        case TYPE_ARRAY: {
          type_array *self = (typeof(self))type;

          type_base *new_element_ref = hir_expand_type(ctx, self->element_ref);
          if (new_element_ref != self->element_ref) {
            type_changed = 1;
          }

          if (type_changed) {
            type_array *new_self = type_array_new(new_element_ref);
            return hir_ctx_type_register(ctx, (type_base *)new_self,
                                         type->type_entry_ref->span);
          }
          return type;
        }
        case TYPE_CALLABLE: {
          type_callable *self = (typeof(self))type;

          type_base *new_ret_ref = hir_expand_type(ctx, self->ret_ref);
          if (new_ret_ref != self->ret_ref) {
            type_changed = 1;
          }

          list_type_ref *new_params = list_type_ref_new();
          for (list_type_ref_it it = list_type_ref_begin(self->params);
               !END(it); NEXT(it)) {
            type_base *param     = GET(it);
            type_base *new_param = hir_expand_type(ctx, param);
            if (param != new_param) {
              type_changed = 1;
            }
            list_type_ref_push_back(new_params, new_param);
          }

          if (type_changed) {
            type_callable *new_self =
                type_callable_new(new_ret_ref, new_params);
            return hir_ctx_type_register(ctx, (type_base *)new_self,
                                         type->type_entry_ref->span);
          }
          list_type_ref_free(new_params);
          return type;
        }
        case TYPE_CLASS_T: {
          type_class_t *self = (typeof(self))type;

          list_type_ref *new_parents = list_type_ref_new();
          for (list_type_ref_it it = list_type_ref_begin(self->parents);
               !END(it); NEXT(it)) {
            type_base *parent     = GET(it);
            type_base *new_parent = hir_expand_type(ctx, parent);
            if (parent != new_parent) {
              type_changed = 1;
            }
            list_type_ref_push_back(new_parents, new_parent);
          }

          if (type_changed) {
            type_class_t *new_self = type_class_t_new(
                strdup(self->id), type_copy_list_ref(self->typenames),
                new_parents);
            return hir_ctx_type_register(ctx, (type_base *)new_self,
                                         type->type_entry_ref->span);
          }
          list_type_ref_free(new_parents);
          return type;
        }
        case TYPE_TYPENAME: {
          type_base *new_self = hir_ctx_type_tmpl_find(ctx, type);

          if (new_self) {
            type_changed = 1;
          }

          if (type_changed) {
            return new_self;
          }
          return type;
        }

        case TYPE_MONO: {
          type_mono *self = (typeof(self))type;

          type_base *new_s_type = hir_expand_type(ctx, self->type_ref);
          if (new_s_type != type) {
            type_changed = 1;
          }

          list_type_ref *new_types = list_type_ref_new();
          for (list_type_ref_it it = list_type_ref_begin(self->types); !END(it);
               NEXT(it)) {
            type_base *type = GET(it);

            type_base *new_type = hir_expand_type(ctx, type);

            if (new_type != type) {
              type_changed = 1;
            }

            list_type_ref_push_back(new_types, new_type);
          }

          if (type_changed) {
            type_mono *new_self = type_mono_new(new_s_type, new_types);

            // char *new_type_s = type_str((type_base *)new_self);
            // info("type changed to %s", new_type_s);
            // free(new_type_s);

            new_self = (type_mono *)hir_ctx_type_register(
                ctx, (type_base *)new_self, type->type_entry_ref->span);

            // add template for further instantiation
            list_type_mono_ref_push_back(ctx->instto_types, new_self);

            // new_type_s = type_str((type_base *)new_self);
            // info("registered %s", new_type_s);
            // free(new_type_s);

            return (type_base *)new_self;
          }
          list_type_ref_free(new_types);
          return type;
        }
        default:
          error("unknown type %d %s", type->kind, type);
          return NULL;
      }
  }
}

static hir_var *hir_expand_templates_var(hir_ctx *ctx, const hir_var *var_t) {
  type_base *type_ref =
      hir_expand_type(ctx, var_t->type_ref ? var_t->type_ref->type : NULL);

  return hir_var_new_typed(span_copy(var_t->base.span),
                           hir_id_copy(var_t->id_hir),
                           type_ref ? type_ref->type_entry_ref : NULL);
}

static hir_param *hir_expand_templates_param(hir_ctx         *ctx,
                                             const hir_param *param_t) {
  type_base *type_ref =
      hir_expand_type(ctx, param_t->type_ref ? param_t->type_ref->type : NULL);

  return hir_param_new_typed(span_copy(param_t->base.span),
                             hir_id_copy(param_t->id_hir),
                             type_ref ? type_ref->type_entry_ref : NULL);
}

static hir_expr_base *hir_expand_templates_expr(hir_ctx             *ctx,
                                                const hir_expr_base *expr_t) {
  if (!expr_t) {
    return NULL;
  }

  const span *span = expr_t->base.span;
  type_base  *type = expr_t->type_ref ? expr_t->type_ref->type : NULL;

  type_base *new_type = hir_expand_type(ctx, type);

  // char *type_s1 = type_str(type);
  // char *type_s2 = type_str(type);
  // debug("expr type %s -> %s", type_s1, type_s2);
  // free(type_s1);
  // free(type_s2);

  type_entry *new_type_ref = new_type ? new_type->type_entry_ref : NULL;

  switch (expr_t->kind) {
    case HIR_EXPR_UNARY: {
      const hir_expr_unary *self = (typeof(self))expr_t;
      return (hir_expr_base *)hir_expr_unary_new_typed(
          span_copy(span), new_type_ref, self->op,
          hir_expand_templates_expr(ctx, self->first));
    }
    case HIR_EXPR_BINARY: {
      const hir_expr_binary *self = (typeof(self))expr_t;
      return (hir_expr_base *)hir_expr_binary_new_typed(
          span_copy(span), new_type_ref, self->op,
          hir_expand_templates_expr(ctx, self->first),
          hir_expand_templates_expr(ctx, self->second));
    }
    case HIR_EXPR_LITERAL: {
      const hir_expr_lit *self = (typeof(self))expr_t;
      return (hir_expr_base *)hir_expr_lit_new_typed(
          span_copy(span), new_type_ref, hir_lit_copy(self->lit));
    }
    case HIR_EXPR_IDENTIFIER: {
      const hir_expr_id *self = (typeof(self))expr_t;
      return (hir_expr_base *)hir_expr_id_new_typed(
          span_copy(span), new_type_ref, hir_id_copy(self->id_hir));
    }
    case HIR_EXPR_CALL: {
      const hir_expr_call *self = (typeof(self))expr_t;
      list_hir_expr       *args = list_hir_expr_new();
      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        list_hir_expr_push_back(args, hir_expand_templates_expr(ctx, GET(it)));
      }
      return (hir_expr_base *)hir_expr_call_new_typed(
          span_copy(span), new_type_ref,
          hir_expand_templates_expr(ctx, self->callee), args);
    }
    case HIR_EXPR_INDEX: {
      const hir_expr_index *self = (typeof(self))expr_t;
      list_hir_expr        *args = list_hir_expr_new();
      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        list_hir_expr_push_back(args, hir_expand_templates_expr(ctx, GET(it)));
      }
      return (hir_expr_base *)hir_expr_index_new_typed(
          span_copy(span), new_type_ref,
          hir_expand_templates_expr(ctx, self->indexed), args);
    }
    case HIR_EXPR_BUILTIN: {
      const hir_expr_builtin *self = (typeof(self))expr_t;
      list_hir_expr          *args = list_hir_expr_new();
      for (list_hir_expr_it it = list_hir_expr_begin(self->args); !END(it);
           NEXT(it)) {
        list_hir_expr_push_back(args, hir_expand_templates_expr(ctx, GET(it)));
      }
      return (hir_expr_base *)hir_expr_builtin_new_typed(
          span_copy(span), new_type_ref, self->kind, args);
    }
  }
  error("unexpected expr kind %d %p", expr_t->kind, expr_t);
  return NULL;
}

static hir_stmt_base *hir_expand_templates_stmt(hir_ctx             *ctx,
                                                const hir_stmt_base *stmt_t) {
  if (!stmt_t) {
    return NULL;
  }

  const span *span = stmt_t->base.span;

  switch (stmt_t->kind) {
    case HIR_STMT_IF: {
      const hir_stmt_if *self = (typeof(self))stmt_t;
      return (hir_stmt_base *)hir_stmt_if_new(
          span_copy(span), hir_expand_templates_expr(ctx, self->cond),
          hir_expand_templates_stmt(ctx, self->je),
          hir_expand_templates_stmt(ctx, self->jz));
    }
    case HIR_STMT_BLOCK: {
      const hir_stmt_block *self  = (typeof(self))stmt_t;
      list_hir_stmt        *stmts = list_hir_stmt_new();
      for (list_hir_stmt_it it = list_hir_stmt_begin(self->stmts); !END(it);
           NEXT(it)) {
        list_hir_stmt_push_back(stmts, hir_expand_templates_stmt(ctx, GET(it)));
      }
      return (hir_stmt_base *)hir_stmt_block_new(span_copy(span), stmts);
    }
    case HIR_STMT_WHILE: {
      const hir_stmt_while *self = (typeof(self))stmt_t;
      return (hir_stmt_base *)hir_stmt_while_new(
          span_copy(span), hir_expand_templates_expr(ctx, self->cond),
          hir_expand_templates_stmt(ctx, self->stmt));
    }
    case HIR_STMT_DO: {
      const hir_stmt_do *self = (typeof(self))stmt_t;
      return (hir_stmt_base *)hir_stmt_do_new(
          span_copy(span), self->positive,
          hir_expand_templates_expr(ctx, self->cond),
          hir_expand_templates_stmt(ctx, self->stmt));
    }
    case HIR_STMT_BREAK: {
      return (hir_stmt_base *)hir_stmt_break_new(span_copy(span));
    }
    case HIR_STMT_EXPR: {
      const hir_stmt_expr *self = (typeof(self))stmt_t;
      return (hir_stmt_base *)hir_stmt_expr_new(
          span_copy(span), hir_expand_templates_expr(ctx, self->expr));
    }
    case HIR_STMT_RETURN: {
      const hir_stmt_return *self = (typeof(self))stmt_t;
      return (hir_stmt_base *)hir_stmt_return_new(
          span_copy(span), hir_expand_templates_expr(ctx, self->expr));
    }
  }
  error("unexpected stmt kind %d %p", stmt_t->kind, stmt_t);
  return NULL;
}

static hir_subroutine *
hir_expand_templates_subroutine(hir_ctx              *ctx,
                                const hir_subroutine *subroutine_t) {
  list_hir_param *params = list_hir_param_new();
  for (list_hir_param_it it = list_hir_param_begin(subroutine_t->params);
       !END(it); NEXT(it)) {
    list_hir_param_push_back(params, hir_expand_templates_param(ctx, GET(it)));
  }

  hir_subroutine_body *body = NULL;

  if (subroutine_t->body) {
    switch (subroutine_t->body->kind) {
      case HIR_SUBROUTINE_BODY_IMPORT:
        body = hir_subroutine_body_new_import(
            hir_lit_copy(subroutine_t->body->body.import.entry),
            hir_lit_copy(subroutine_t->body->body.import.lib));
        break;
      case HIR_SUBROUTINE_BODY_BLOCK: {
        list_hir_var *vars = list_hir_var_new();
        for (list_hir_var_it it =
                 list_hir_var_begin(subroutine_t->body->body.block.vars);
             !END(it); NEXT(it)) {
          list_hir_var_push_back(vars, hir_expand_templates_var(ctx, GET(it)));
        }
        body = hir_subroutine_body_new_block(
            vars,
            (hir_stmt_block *)hir_expand_templates_stmt(
                ctx, (hir_stmt_base *)subroutine_t->body->body.block.block));
        break;
      }
      default:
        error("unexpected body kind %d %p", subroutine_t->body->kind,
              subroutine_t->body);
        break;
    }
  }

  return hir_subroutine_new_typed(
      span_copy(subroutine_t->base.span), hir_id_copy(subroutine_t->id_hir),
      params,
      hir_expand_type(ctx, subroutine_t->type_ref->type)->type_entry_ref,
      subroutine_t->spec, body);
}

static hir_class *hir_expand_templates_class(hir_ctx   *ctx,
                                             type_mono *mono_type) {

  const hir_class *class_t =
      (typeof(class_t))hir_get_hir_template(ctx, mono_type);
  if (!class_t) {
    return NULL;
  }

  // char *type_s = type_str((type_base *)mono_type);
  // info("expanding type %s", type_s);
  // free(type_s);

  // setup templates that need to be replaced
  ctx->map_type_tmpl = hashset_hir_type_map_new();

  type_class_t *class_t_type = (typeof(class_t_type))class_t->type_ref->type;

  for (list_type_ref_it fit = list_type_ref_begin(class_t_type->typenames),
                        tit = list_type_ref_begin(mono_type->types);
       !END(tit); NEXT(fit), NEXT(tit)) {
    type_base *from_type = GET(fit);
    type_base *to_type   = GET(tit);
    hashset_hir_type_map_insert(ctx->map_type_tmpl,
                                hir_type_map_new(from_type, to_type));

    // char *type_f = type_str(GET(fit));
    // char *type_t = type_str(GET(tit));
    // info("%s -> %s", type_f, type_t);
    // if (type_f)
    //   free(type_f);
    // if (type_t)
    //   free(type_t);
  }

  // expansion of parents may create new template (because typenames are
  // expanded), that's why need to update mono
  // doesn't create new, because other references should stay valid
  {
    int type_changed = 0;

    type_base *new_class_t_type =
        hir_expand_type(ctx, (type_base *)class_t_type);

    if (new_class_t_type != (type_base *)class_t_type) {
      type_changed = 1;
    }

    if (type_changed) {
      mono_type->type_ref = (type_base *)new_class_t_type;
    }
  }

  hir_class *hir_class = hir_class_new_typed(
      span_copy(class_t->base.span), mono_type->base.type_entry_ref,
      list_hir_var_new(), list_hir_method_new());

  for (list_hir_var_it it = list_hir_var_begin(class_t->fields); !END(it);
       NEXT(it)) {
    hir_var *var = hir_expand_templates_var(ctx, GET(it));
    if (var) {
      list_hir_var_push_back(hir_class->fields, var);
    }
  }

  for (list_hir_method_it it = list_hir_method_begin(class_t->methods);
       !END(it); NEXT(it)) {
    const hir_method *method_t = GET(it);
    hir_subroutine   *sub =
        hir_expand_templates_subroutine(ctx, method_t->subroutine);
    if (sub) {
      list_hir_method_push_back(hir_class->methods,
                                hir_method_new(span_copy(method_t->base.span),
                                               method_t->modifier, sub));
    }
  }

  hashset_hir_type_map_free(ctx->map_type_tmpl);

  return hir_class;
}

static void hir_expand_templates_all(hir_ctx *ctx) {
  size_t i = 0;
  while (!list_type_mono_ref_empty(ctx->instto_types)) {
    type_mono *mono = list_type_mono_ref_pop_front(ctx->instto_types);

    if (++i == 100) {
      break;
    }

    union {
      void *result;
      hir_class *class;
    } hir;

    hashset_type_ref_it found =
        hashset_type_ref_find(ctx->insted_types, (type_base *)mono);

    if (!END(found)) {
      continue;
    }

#ifndef NDEBUG
    char *type_from_s = type_str((type_base *)mono);
#endif

    switch (mono->type_ref->kind) {
      case TYPE_CLASS_T:
        hir.class = hir_expand_templates_class(ctx, mono);
        break;

      default: {

#ifndef NDEBUG
        error("expansion of type %s is unsupported", type_from_s);
        if (type_from_s) {
          free(type_from_s);
        }
#endif

        continue;
      }
    }

#ifndef NDEBUG
    char *type_to_s = type_str((type_base *)mono);
    debug("expanded %s", type_from_s);
    if (strcmp(type_from_s, type_to_s)) {
      debug("     --> %s", type_to_s);
    }

    if (type_from_s) {
      free(type_from_s);
    }
    if (type_to_s) {
      free(type_to_s);
    }
#endif

    if (hir.result) {
      hashset_type_ref_insert(ctx->insted_types, (type_base *)mono);
      list_hir_class_push_back(ctx->hir_classes, hir.result);
    }
  }
}

hir_expand_templates_result hir_expand_templates(hir        *hir,
                                                 type_table *type_table) {
  hir_expand_templates_result result = {
      .exceptions = list_exception_new(),
  };

  hir_ctx ctx;

  // will append classes and exceptions
  hir_ctx_init(&ctx, type_table, result.exceptions, hir->classes);

  hir_setup_ctx(&ctx);
  hir_setup_ctx_hir_templates(&ctx, hir_filter_templated_hir_classes(hir));
  hir_setup_ctx_instto_types(&ctx, hir_get_mono_full_class_types(type_table));

  hir_ctx_debug(&ctx, "before expansion");

  hir_expand_templates_all(&ctx);

  hir_ctx_debug(&ctx, "after expansion");

  hir_ctx_deinit(&ctx);

  return result;
}
