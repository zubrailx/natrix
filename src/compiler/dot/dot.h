#pragma once

#include <stdio.h>

#include "compiler/ast/ast.h"
#include "compiler/hir/hir.h"
#include "compiler/mir/mir.h"

struct dot_string {
  char  *chars;
  size_t len;
};

typedef struct dot_string dot_string;

struct dot_string *dot_string_new(char *chars, size_t len);
void               dot_string_free(dot_string *);

dot_string *dot_ast(const ast *ast);
dot_string *dot_cfg(const mir *mir, const hir *hir,
                    const type_table   *type_table,
                    const symbol_table *symbol_table, const mir_subroutine *sub,
                    int option_add_expr);
dot_string *dot_cg(const mir *mir, const type_table *type_table,
                   const symbol_table *symbol_table, const mir_subroutine *sub);
