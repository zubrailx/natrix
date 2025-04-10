#include <criterion/criterion.h>

#include "compiler/hir/str.h"
#include "compiler/hir/type.h"
#include "util/log.h"

Test(hir_type, test1) {
  list_hir_type *lsv_t = list_hir_type_new();
  list_hir_type_push_back(
      lsv_t, (hir_type_base *)hir_type_custom_new(NULL, strdup("T"), NULL));

  hir_type_custom *lsv = hir_type_custom_new(NULL, strdup("B"), lsv_t);

  list_hir_type *rsv_tt = list_hir_type_new();
  list_hir_type_push_back(
      rsv_tt, (hir_type_base *)hir_type_base_new(NULL, HIR_TYPE_INT));

  list_hir_type *rsv_t = list_hir_type_new();
  list_hir_type_push_back(
      rsv_t, (hir_type_base *)hir_type_custom_new(NULL, strdup("B"), rsv_tt));

  hir_type_custom *rsv = hir_type_custom_new(NULL, strdup("B"), rsv_t);

  char *lsv_s = hir_type_str((hir_type_base *)lsv);
  char *rsv_s = hir_type_str((hir_type_base *)rsv);
  debug("test = %s <-> %s == %d", lsv_s, rsv_s, hir_type_cmp((hir_type_base *)lsv, (hir_type_base *)rsv));
  free(lsv_s);
  free(rsv_s);


  hir_type_free((hir_type_base *)lsv);
  hir_type_free((hir_type_base *)rsv);
}
