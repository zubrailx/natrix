#pragma once

#include "compiler/exception/list.h"
#include "compiler/hir/hir.h"
#include "compiler/symbol_table/symbol_table.h"

typedef struct hir_bind_symbols_result_struct {
  symbol_table   *symbol_table;
  list_exception *exceptions;
} hir_bind_symbols_result;

hir_bind_symbols_result hir_bind_symbols(hir *hir);
