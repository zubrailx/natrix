#pragma once

#include "compiler/hir/node_class.h"
#include "lower_ast.h"

#include "compiler/hir/node_subroutine.h"
#include <antlr3basetree.h>

typedef hir_lower_ast_ctx hir_ctx;

// TERMINALS
hir_lit *hir_build_str(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_lit *hir_build_rune(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_lit *hir_build_hex(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_lit *hir_build_bits(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_lit *hir_build_dec(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_lit *hir_build_bool(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_id  *hir_build_id(hir_ctx *ctx, ANTLR3_BASE_TREE *node);

// TYPES
hir_type_base   *hir_build_type_bool(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_byte(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_int(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_uint(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_long(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_ulong(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_char(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_string(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_void(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_base   *hir_build_type_any(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_type_custom *hir_build_type_custom(hir_ctx *ctx, hir_id *id,
                                       list_hir_type *templates);
hir_type_array  *hir_build_type_array(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                      ANTLR3_BASE_TREE *dimensions,
                                      hir_type_base    *elem_type);

// PARAMS & VARS
hir_param    *hir_build_param(hir_ctx *ctx, hir_id *id, hir_type_base *type);
list_hir_var *hir_build_var_entry(hir_ctx *ctx, list_hir_id *ids,
                                  hir_type_base *type);
void          hir_build_var_list_extend(hir_ctx *ctx, list_hir_var *first,
                                        list_hir_var *second);

// SUBROUTINE
hir_subroutine      *hir_build_signature(hir_ctx *ctx, ANTLR3_BASE_TREE *root,
                                         hir_id *id, list_hir_param *params,
                                         hir_type_base *ret_type);
hir_subroutine      *hir_build_func(hir_ctx *ctx, hir_subroutine *subroutine,
                                    hir_subroutine_body *body,
                                    ANTLR3_BASE_TREE    *sp_extern);
hir_subroutine_body *hir_build_func_body_block(hir_ctx *ctx, list_hir_var *vars,
                                               hir_stmt_block *body);
hir_subroutine_body *hir_build_func_body_import(hir_ctx *ctx, hir_lit *lib,
                                                hir_lit *entry);

// CLASS
hir_method *hir_build_method(hir_ctx *ctx, ANTLR3_BASE_TREE *mod_t,
                             hir_method_modifier_enum modifier,
                             hir_subroutine *func, const hir_id *class_id_ref,
                             const list_hir_id *typenames_ref);
hir_class  *hir_build_class(hir_ctx *ctx, ANTLR3_BASE_TREE *root, hir_id *id,
                            list_hir_id *typenames, list_hir_type *parents,
                            list_hir_var *fields, list_hir_method *methods);

// EXPRESSIONS
hir_expr_lit     *hir_build_expr_lit(hir_ctx *ctx, hir_lit *lit);
hir_expr_id      *hir_build_expr_id(hir_ctx *ctx, hir_id *id);
hir_expr_index   *hir_build_expr_indexer(hir_ctx *ctx, hir_expr_base *indexed,
                                         list_hir_expr *args);
hir_expr_call    *hir_build_expr_caller(hir_ctx *ctx, hir_expr_base *indexed,
                                        list_hir_expr *args);
hir_expr_unary   *hir_build_expr_unary(hir_ctx *ctx, hir_expr_base *first,
                                       ANTLR3_BASE_TREE   *op_node,
                                       hir_expr_unary_enum op);
hir_expr_binary  *hir_build_expr_binary(hir_ctx *ctx, hir_expr_base *first,
                                        hir_expr_base       *second,
                                        ANTLR3_BASE_TREE    *op_node,
                                        hir_expr_binary_enum op);
hir_expr_binary  *hir_build_expr_member(hir_ctx *ctx, hir_expr_base *first,
                                        hir_id           *second,
                                        ANTLR3_BASE_TREE *op_node);
hir_expr_builtin *hir_build_expr_builtin(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                         hir_expr_builtin_enum kind,
                                         hir_type_base        *type,
                                         list_hir_expr        *expr);

// STATEMENTS
hir_stmt_if     *hir_build_stmt_if(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                   hir_expr_base *cond, hir_stmt_base *je,
                                   hir_stmt_base *jz);
hir_stmt_block  *hir_build_stmt_block(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                      list_hir_stmt *stmts);
hir_stmt_while  *hir_build_stmt_while(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                      hir_expr_base *cond, hir_stmt_base *stmt);
hir_stmt_do     *hir_build_stmt_do(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                   int positive, hir_expr_base *cond,
                                   hir_stmt_base *stmt);
hir_stmt_break  *hir_build_stmt_break(hir_ctx *ctx, ANTLR3_BASE_TREE *node);
hir_stmt_expr   *hir_build_stmt_expr(hir_ctx *ctx, hir_expr_base *expr);
hir_stmt_return *hir_build_stmt_return(hir_ctx *ctx, ANTLR3_BASE_TREE *node,
                                       hir_expr_base *expr);

// HIR
void hir_build_hir_func(hir_ctx *ctx, hir_subroutine *func);
void hir_build_hir_class(hir_ctx *ctx, hir_class *class);
