#pragma once

#include "compiler/hir/node_class.h"
#include "compiler/hir/node_subroutine.h"

typedef struct hir_struct {
  list_hir_class      *classes;
  list_hir_subroutine *subroutines;
} hir;

hir *hir_new();
void hir_free(hir *self);
