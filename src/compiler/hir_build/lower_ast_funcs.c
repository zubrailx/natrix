#include "lower_ast_funcs.h"

#include "compiler/antlr3/antlr3.h"
#include "compiler/hir/str.h"
#include "compiler/hir_build/exception.h"
#include "compiler/span/span.h"
// #include "compiler/span/str.h"
// #include "util/log.h"
#include "util/macro.h"
#include <antlr3basetree.h>
#include <antlr3commontoken.h>
#include <errno.h>

static span *hir_span_new(ANTLR3_BASE_TREE *first, const char *source) {
  if (!first || !first->getToken(first)) {
    // debug("span is set to default");
    return span_new(source, 0, 0, 0, 0);
  }
  ANTLR3_COMMON_TOKEN *first_token = first->getToken(first);

  span *span = span_new(source, first_token->line, first_token->line,
                        first_token->getCharPositionInLine(first_token),
                        first_token->getCharPositionInLine(first_token) +
                            (first_token->stop - first_token->start + 1));
  return span;
}

static span *hir_span_new_range(const span *first, const span *second) {
  span *span = span_new(first->source_ref, first->line_start, second->line_end,
                        first->pos_start, second->pos_end);
  return span;
}

static long hir_build_str2long(const char *str, int base, char **err_str_out) {
  char errbuf[256];
  errbuf[0] = '\0';

  errno        = 0;
  char *endptr = NULL;
  long  number = strtol(str, &endptr, base);

  /* test return to number and errno values */
  if (str == endptr)
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu invalid (no digits found, 0 returned)\n", number);
  else if (errno == ERANGE && number == LONG_MIN)
    snprintf(errbuf, sizeof(errbuf) - 1, "%lu invalid (underflow occurred)",
             number);
  else if (errno == ERANGE && number == LONG_MAX)
    snprintf(errbuf, sizeof(errbuf) - 1, "%lu invalid (overflow occurred)",
             number);
  else if (errno == EINVAL) /* not in all c99 implementations - gcc OK */
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu invalid (base contains unsupported value)", number);
  else if (errno != 0 && number == 0)
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu invalid (unspecified error occurred)", number);
  else if (errno == 0 && str && *endptr != 0)
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu valid (but additional characters remain)", number);

  if (errbuf[0] != '\0') {
    *err_str_out = malloc(sizeof(errbuf));
    memcpy(*err_str_out, errbuf, sizeof(errbuf));
  }

  return number;
}

static ulong hir_build_str2ulong(const char *str, int base,
                                 char **err_str_out) {
  char errbuf[256];
  errbuf[0] = '\0';

  errno        = 0;
  char *endptr = NULL;
  ulong number = strtoul(str, &endptr, base);

  /* test return to number and errno values */
  if (str == endptr)
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu invalid (no digits found, 0 returned)\n", number);
  else if (errno == ERANGE && number == 0)
    snprintf(errbuf, sizeof(errbuf) - 1, "%lu invalid (underflow occurred)",
             number);
  else if (errno == ERANGE && number == ULONG_MAX)
    snprintf(errbuf, sizeof(errbuf) - 1, "%lu invalid (overflow occurred)",
             number);
  else if (errno == EINVAL) /* not in all c99 implementations - gcc OK */
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu invalid (base contains unsupported value)", number);
  else if (errno != 0 && number == 0)
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu invalid (unspecified error occurred)", number);
  else if (errno == 0 && str && *endptr != 0)
    snprintf(errbuf, sizeof(errbuf) - 1,
             "%lu valid (but additional characters remain)", number);

  if (errbuf[0] != '\0') {
    *err_str_out = malloc(sizeof(errbuf));
    memcpy(*err_str_out, errbuf, sizeof(errbuf));
  }

  return number;
}

// ERRORS
static void hir_error_add(hir_ctx *ctx, exception_subtype_hir subtype,
                          const span *span, const char *format, ...) {

  va_list args;
  va_start(args, format);
  hir_exception_add_error_v(ctx->exceptions, subtype, span, format, args);
  va_end(args);
}

// IDENTIFIER
hir_id *hir_build_id(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  const char *name = ANTLR3_CHARS(node);
  return hir_id_new(hir_span_new(node, ctx->ast_cur_ref->name_ref),
                    strdup(name));
}

// LITERALS
hir_lit *hir_build_str(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span       *span  = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  const char *chars = ANTLR3_CHARS(node);
  size_t      len   = strlen(chars);
  char       *data  = malloc(len - 1);
  strncpy(data, chars + 1, len - 2);
  data[len - 2] = '\0';
  return hir_lit_new(span, hir_type_base_new(span_copy(span), HIR_TYPE_STRING),
                     (hir_lit_u){.v_str = data});
}

hir_lit *hir_build_rune(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span       *span  = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  const char *chars = ANTLR3_CHARS(node);
  size_t      len   = strlen(chars);
  char       *data  = malloc(len - 1);
  strncpy(data, chars + 1, len - 2);
  data[len - 2] = '\0';
  return hir_lit_new(span, hir_type_base_new(span_copy(span), HIR_TYPE_CHAR),
                     (hir_lit_u){.v_char = data});
}

hir_lit *hir_build_hex(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span       *span  = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  const char *chars = ANTLR3_CHARS(node) + 2;

  char *err_msg = NULL;
  long  value   = hir_build_str2long(chars, 16, &err_msg);
  if (err_msg) {
    free(err_msg);
    err_msg      = NULL;
    ulong uvalue = hir_build_str2ulong(chars, 16, &err_msg);
    if (err_msg) {
      hir_error_add(ctx, EXCEPTION_HIR_HEX_VALIDATION, span, err_msg);
      free(err_msg);
      return NULL;
    }
    return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_ULONG),
                       (hir_lit_u){.v_ulong = uvalue});
  }
  return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_LONG),
                     (hir_lit_u){.v_long = value});
}

hir_lit *hir_build_bits(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span       *span  = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  const char *chars = ANTLR3_CHARS(node) + 2;

  char *err_msg = NULL;
  long  value   = hir_build_str2long(chars, 2, &err_msg);
  if (err_msg) {
    free(err_msg);
    err_msg      = NULL;
    ulong uvalue = hir_build_str2ulong(chars, 2, &err_msg);
    if (err_msg) {
      hir_error_add(ctx, EXCEPTION_HIR_BITS_VALIDATION, span, err_msg);
      free(err_msg);
      return NULL;
    }
    return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_ULONG),
                       (hir_lit_u){.v_ulong = uvalue});
  }
  return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_LONG),
                     (hir_lit_u){.v_long = value});
}

hir_lit *hir_build_dec(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span       *span  = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  const char *chars = ANTLR3_CHARS(node);

  char *err_msg = NULL;
  long  value   = hir_build_str2long(chars, 10, &err_msg);
  if (err_msg) {
    free(err_msg);
    err_msg      = NULL;
    ulong uvalue = hir_build_str2ulong(chars, 10, &err_msg);
    if (err_msg) {
      hir_error_add(ctx, EXCEPTION_HIR_DEC_VALIDATION, span, err_msg);
      free(err_msg);
      return NULL;
    }
    return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_ULONG),
                       (hir_lit_u){.v_ulong = uvalue});
  }
  return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_LONG),
                     (hir_lit_u){.v_long = value});
}

hir_lit *hir_build_bool(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span       *span  = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  const char *chars = ANTLR3_CHARS(node);

  hir_lit_u u;
  if (!strcmp("true", chars)) {
    u.v_bool = 1;
  } else if (!strcmp("false", chars)) {
    u.v_bool = 0;
  } else {
    hir_error_add(ctx, EXCEPTION_HIR_BOOL_VALIDATION, span,
                  "%s is invalid boolean const", chars);
    return NULL;
  }
  return hir_lit_new(span, hir_type_base_new(NULL, HIR_TYPE_BOOL), u);
}

// TYPES
hir_type_base *hir_build_type_bool(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_BOOL);
}

hir_type_base *hir_build_type_byte(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_BYTE);
}

hir_type_base *hir_build_type_int(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_INT);
}

hir_type_base *hir_build_type_uint(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_UINT);
}

hir_type_base *hir_build_type_long(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_LONG);
}

hir_type_base *hir_build_type_ulong(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_ULONG);
}

hir_type_base *hir_build_type_char(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_CHAR);
}

hir_type_base *hir_build_type_string(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_STRING);
}

hir_type_base *hir_build_type_void(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_VOID);
}

hir_type_base *hir_build_type_any(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_type_base_new(span, HIR_TYPE_ANY);
}

hir_type_custom *hir_build_type_custom(hir_ctx *ctx, hir_id *id,
                                       list_hir_type *templates) {
  UNUSED(ctx);
  hir_id_data      id_data = hir_id_pop(id);
  hir_type_custom *custom =
      hir_type_custom_new(id_data.span, id_data.name, templates);

  char *str = hir_type_str((hir_type_base *)custom);
  free(str);

  return custom;
}

hir_type_array *hir_build_type_array(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                     ANTLR3_BASE_TREE *dimensions_node,
                                     hir_type_base    *elem_type) {
  span  *root       = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  size_t dimensions = ANTLR3_SIZE(dimensions_node);
  span  *span_m     = hir_span_new_range(root, elem_type->span);
  span_free(root);

  hir_type_base *type_root = elem_type;
  for (size_t i = 0; i < dimensions; ++i) {
    type_root =
        (hir_type_base *)hir_type_array_new(span_copy(span_m), type_root);
  }
  return hir_type_array_new(span_m, type_root);
}

hir_param *hir_build_param(hir_ctx *ctx, hir_id *id, hir_type_base *type) {
  UNUSED(ctx);
  span *span;
  if (type) {
    span = hir_span_new_range(id->base.span, type->span);
  } else {
    span = span_copy(id->base.span);
  }
  return hir_param_new(span, id, type);
}

hir_subroutine *hir_build_signature(hir_ctx *ctx, ANTLR3_BASE_TREE *root,
                                    hir_id *id, list_hir_param *params,
                                    hir_type_base *ret_type) {
  span *root_span = hir_span_new(root, ctx->ast_cur_ref->name_ref);
  span *span;
  if (ret_type) {
    span = hir_span_new_range(root_span, ret_type->span);
  } else if (list_hir_param_size(params) > 0) {
    span =
        hir_span_new_range(root_span, list_hir_param_back(params)->base.span);
  } else {
    span = hir_span_new_range(root_span, id->base.span);
  }
  span_free(root_span);
  // debug("%zu:%zu %zu:%zu", span->line_start, span->pos_start, span->line_end,
  // span->pos_end);
  return hir_subroutine_new(span, id, params, ret_type,
                            HIR_SUBROUTINE_SPEC_EMPTY, NULL);
}

list_hir_var *hir_build_var_entry(hir_ctx *ctx, list_hir_id *ids,
                                  hir_type_base *type) {
  UNUSED(ctx);
  list_hir_var *var_list = list_hir_var_new();

  size_t id_size = list_hir_id_size(ids);
  for (size_t i = 1; i < id_size; ++i) {
    hir_id *id = list_hir_id_pop_front(ids);
    list_hir_var_push_back(var_list, hir_var_new(span_copy(id->base.span), id,
                                                 hir_type_copy(type)));
  }
  if (id_size > 0) {
    hir_id *id = list_hir_id_pop_front(ids);
    list_hir_var_push_back(var_list,
                           hir_var_new(span_copy(id->base.span), id, type));
  }

  list_hir_id_free(ids);
  return var_list;
}

void hir_build_var_list_extend(hir_ctx *ctx, list_hir_var *first,
                               list_hir_var *second) {
  UNUSED(ctx);
  size_t size = list_hir_var_size(second);
  for (size_t i = 0; i < size; ++i) {
    hir_var *var = list_hir_var_pop_front(second);
    list_hir_var_push_back(first, var);
  }
  list_hir_var_free(second);
}

hir_subroutine *hir_build_func(hir_ctx *ctx, hir_subroutine *subroutine,
                               hir_subroutine_body *body,
                               ANTLR3_BASE_TREE    *sp_extern) {
  UNUSED(ctx);

  // include specifiers into subroutine span
  if (sp_extern) {
    span *span_extern = hir_span_new(sp_extern, ctx->ast_cur_ref->name_ref);
    span *span        = hir_span_new_range(span_extern, subroutine->base.span);
    span_free(span_extern);
    span_free(subroutine->base.span);
    subroutine->base.span = span;
  }

  if (sp_extern) {
    subroutine->spec |= HIR_SUBROUTINE_SPEC_EXTERN;
  }

  subroutine->body = body;
  return subroutine;
}

hir_subroutine_body *hir_build_func_body_block(hir_ctx *ctx, list_hir_var *vars,
                                               hir_stmt_block *body) {
  UNUSED(ctx);
  return hir_subroutine_body_new_block(vars, body);
}

hir_subroutine_body *hir_build_func_body_import(hir_ctx *ctx, hir_lit *lib,
                                                hir_lit *entry) {
  UNUSED(ctx);
  return hir_subroutine_body_new_import(entry, lib);
}

// CLASS
hir_method *hir_build_method(hir_ctx *ctx, ANTLR3_BASE_TREE *mod_t,
                             hir_method_modifier_enum modifier,
                             hir_subroutine *func, const hir_id *class_id_ref,
                             const list_hir_id *typenames_ref) {
  UNUSED(ctx);

  // add implicit this first param
  list_hir_type *typenames = list_hir_type_new();
  for (list_hir_id_it it = list_hir_id_begin(typenames_ref); !END(it);
       NEXT(it)) {
    list_hir_type_push_back(typenames, (hir_type_base *)hir_type_custom_new(
                                           NULL, strdup(GET(it)->name), NULL));
  }

  list_hir_param_push_front(
      func->params,
      hir_param_new(NULL, hir_id_new(NULL, strdup("this")),
                    (hir_type_base *)hir_type_custom_new(
                        NULL, strdup(class_id_ref->name), typenames)));

  if (!mod_t || modifier == HIR_METHOD_MODIFIER_ENUM_EMPTY) {
    span *span = span_copy(func->base.span);
    return hir_method_new(span, modifier, func);
  } else {
    span *mod_span = hir_span_new(mod_t, ctx->ast_cur_ref->name_ref);
    span *span     = hir_span_new_range(mod_span, func->base.span);
    span_free(mod_span);
    return hir_method_new(span, modifier, func);
  }
}

hir_class *hir_build_class(hir_ctx *ctx, ANTLR3_BASE_TREE *root, hir_id *id,
                           list_hir_id *typenames, list_hir_type *parents,
                           list_hir_var *fields, list_hir_method *methods) {
  span *root_span = hir_span_new(root, ctx->ast_cur_ref->name_ref);
  span *span;
  if (list_hir_type_size(parents)) {
    span = hir_span_new_range(root_span, list_hir_type_back(parents)->span);
  } else if (list_hir_id_size(typenames)) {
    span =
        hir_span_new_range(root_span, list_hir_id_back(typenames)->base.span);
  } else {
    span = hir_span_new_range(root_span, id->base.span);
  }
  span_free(root_span);

  return hir_class_new(span, id, typenames, parents, fields, methods);
}

// EXPRESSIONS
hir_expr_lit *hir_build_expr_lit(hir_ctx *ctx, hir_lit *lit) {
  UNUSED(ctx);
  return hir_expr_lit_new(span_copy(lit->base.span), NULL, lit);
}

hir_expr_id *hir_build_expr_id(hir_ctx *ctx, hir_id *id) {
  UNUSED(ctx);
  return hir_expr_id_new(span_copy(id->base.span), NULL, id);
}

hir_expr_index *hir_build_expr_indexer(hir_ctx *ctx, hir_expr_base *indexed,
                                       list_hir_expr *args) {
  UNUSED(ctx);
  span *span;
  if (list_hir_expr_size(args) > 0) {
    span = hir_span_new_range(indexed->base.span,
                              list_hir_expr_back(args)->base.span);
  } else {
    span = span_copy(indexed->base.span);
  }
  return hir_expr_index_new(span, NULL, indexed, args);
}

hir_expr_call *hir_build_expr_caller(hir_ctx *ctx, hir_expr_base *indexed,
                                     list_hir_expr *args) {
  UNUSED(ctx);
  span *span;
  if (list_hir_expr_size(args) > 0) {
    span = hir_span_new_range(indexed->base.span,
                              list_hir_expr_back(args)->base.span);
  } else {
    span = span_copy(indexed->base.span);
  }
  return hir_expr_call_new(span, NULL, indexed, args);
}

hir_expr_unary *hir_build_expr_unary(hir_ctx *ctx, hir_expr_base *first,
                                     ANTLR3_BASE_TREE   *op_node,
                                     hir_expr_unary_enum op) {
  span *op_span = hir_span_new(op_node, ctx->ast_cur_ref->name_ref);
  span *span;
  if (op_span->line_start < first->base.span->line_start) {
    span = hir_span_new_range(op_span, first->base.span);
  } else {
    span = hir_span_new_range(first->base.span, op_span);
  }
  span_free(op_span);
  return hir_expr_unary_new(span, NULL, op, first);
}

hir_expr_binary *hir_build_expr_binary(hir_ctx *ctx, hir_expr_base *first,
                                       hir_expr_base       *second,
                                       ANTLR3_BASE_TREE    *op_node,
                                       hir_expr_binary_enum op) {
  UNUSED(ctx);
  UNUSED(op_node);
  return hir_expr_binary_new(
      hir_span_new_range(first->base.span, second->base.span), NULL, op, first,
      second);
}

// convert id to literal because it is not actually a symbol
hir_expr_binary *hir_build_expr_member(hir_ctx *ctx, hir_expr_base *first,
                                       hir_id           *second,
                                       ANTLR3_BASE_TREE *op_node) {
  UNUSED(ctx);
  UNUSED(op_node);

  hir_id_data data      = hir_id_pop(second);
  span       *expr_span = hir_span_new_range(first->base.span, data.span);

  return hir_expr_binary_new(
      expr_span, NULL, HIR_EXPR_BINARY_MEMBER, first,
      (hir_expr_base *)hir_expr_lit_new(
          NULL, NULL,
          hir_lit_new(data.span, hir_type_base_new(NULL, HIR_TYPE_STRING),
                      (hir_lit_u){.v_str = data.name})));
}

hir_expr_builtin *hir_build_expr_builtin(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                         hir_expr_builtin_enum kind,
                                         hir_type_base        *type,
                                         list_hir_expr        *args) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  span *span;

  if (list_hir_expr_size(args)) {
    span = hir_span_new_range(node_span, list_hir_expr_back(args)->base.span);
    span_free(node_span);
  } else if (type) {
    span = hir_span_new_range(node_span, type->span);
    span_free(node_span);
  } else {
    span = node_span;
  }

  return hir_expr_builtin_new(span, type, kind, args);
}

// STATEMENTS
hir_stmt_if *hir_build_stmt_if(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                               hir_expr_base *cond, hir_stmt_base *je,
                               hir_stmt_base *jz) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  span *span;
  if (jz) {
    span = hir_span_new_range(node_span, jz->base.span);
  } else {
    span = hir_span_new_range(node_span, je->base.span);
  }
  span_free(node_span);
  // debug("%zu:%zu %zu:%zu", span->line_start, span->pos_start,
  // span->line_end, span->pos_end);
  return hir_stmt_if_new(span, cond, je, jz);
}

hir_stmt_block *hir_build_stmt_block(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                     list_hir_stmt *stmts) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  span *span;
  if (list_hir_stmt_size(stmts) > 0) {
    span = hir_span_new_range(node_span, list_hir_stmt_back(stmts)->base.span);
    span_free(node_span);
  } else {
    span = node_span;
  }
  return hir_stmt_block_new(span, stmts);
}

hir_stmt_while *hir_build_stmt_while(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                     hir_expr_base *cond, hir_stmt_base *stmt) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  span *span      = hir_span_new_range(node_span, stmt->base.span);
  span_free(node_span);
  return hir_stmt_while_new(span, cond, stmt);
}

hir_stmt_do *hir_build_stmt_do(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                               int positive, hir_expr_base *cond,
                               hir_stmt_base *stmt) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  span *span      = hir_span_new_range(node_span, cond->base.span);
  span_free(node_span);
  return hir_stmt_do_new(span, positive, cond, stmt);
}

hir_stmt_break *hir_build_stmt_break(hir_ctx *ctx, ANTLR3_BASE_TREE *node) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  return hir_stmt_break_new(node_span);
}

hir_stmt_expr *hir_build_stmt_expr(hir_ctx *ctx, hir_expr_base *expr) {
  UNUSED(ctx);
  return hir_stmt_expr_new(span_copy(expr->base.span), expr);
}

hir_stmt_return *hir_build_stmt_return(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                       hir_expr_base *expr) {
  span *node_span = hir_span_new(node, ctx->ast_cur_ref->name_ref);
  span *span      = hir_span_new_range(node_span, expr->base.span);
  span_free(node_span);
  return hir_stmt_return_new(span, expr);
}

// HIR
void hir_build_hir_func(hir_ctx *ctx, hir_subroutine *func) {
  list_hir_subroutine_push_back(ctx->hir_ref->subroutines, func);
}

void hir_build_hir_class(hir_ctx *ctx, hir_class *class) {
  list_hir_class_push_back(ctx->hir_ref->classes, class);
}
