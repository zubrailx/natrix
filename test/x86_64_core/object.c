#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/proxy/value/object.h"
#include "x86_64_core/proxy/value/void.h"
#include "x86_64_core/value.h"

Test(x86_64_object, test1) {
  x86_64_value value1;
  x86_64_value value2;

  uint64_t                    symbols_count = 0;
  x86_64_data_object_symbols *symbols =
      malloc(sizeof(x86_64_data_object_symbols) +
             sizeof(x86_64_data_object_symbol) * symbols_count);

  symbols->count = symbols_count;

  __x86_64_proxy_object_init(&value1, symbols);

  value1.op_tbl->op_repr(&value2, &value1);
  debug("object: %s, type %d", value2.data_ptr, value1.type);
  value2.op_tbl->op_drop(&value2);

  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);

  free(symbols);
}

Test(x86_64_object, test2) {
  x86_64_value value1;
  x86_64_value value2;

  uint64_t                    symbols_count = 2;
  x86_64_data_object_symbols *symbols =
      malloc(sizeof(x86_64_data_object_symbols) +
             sizeof(x86_64_data_object_symbol) * symbols_count);

  symbols->count = symbols_count;
  symbols->symbols[0] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"first"};
  symbols->symbols[1] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"second"};

  __x86_64_proxy_object_init(&value1, symbols);

  value1.op_tbl->op_repr(&value2, &value1);
  debug("object: %s, type %d", value2.data_ptr, value1.type);
  value2.op_tbl->op_drop(&value2);

  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);

  free(symbols);
}

Test(x86_64_object, test3_index) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  uint64_t                    symbols_count = 2;
  x86_64_data_object_symbols *symbols =
      malloc(sizeof(x86_64_data_object_symbols) +
             sizeof(x86_64_data_object_symbol) * symbols_count);

  symbols->count = symbols_count;
  symbols->symbols[0] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"first"};
  symbols->symbols[1] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"second"};

  __x86_64_proxy_object_init(&value1, symbols);

  __x86_64_proxy_int_init(&value4, 0);

  value1.op_tbl->op_index_ref(&value3, &value1, &value4);
  value3.op_tbl->op_assign(&value3, &value4);

  value3.op_tbl->op_repr(&value2, &value3);
  debug("object_index: %s, type %d", value2.data_ptr, value3.type);
  value2.op_tbl->op_drop(&value2);

  __x86_64_print(&value1);

  value1.op_tbl->op_drop(&value1);
  value3.op_tbl->op_drop(&value3);

  free(symbols);
}

Test(x86_64_object, test4_member) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;

  uint64_t                    symbols_count = 2;
  x86_64_data_object_symbols *symbols =
      malloc(sizeof(x86_64_data_object_symbols) +
             sizeof(x86_64_data_object_symbol) * symbols_count);

  symbols->count = symbols_count;
  symbols->symbols[0] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"first"};
  symbols->symbols[1] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"second"};

  __x86_64_proxy_object_init(&value1, symbols);

  __x86_64_proxy_int_init(&value4, 0);

  value1.op_tbl->op_member_ref(&value3, &value1, (const uint8_t *)"second");
  value3.op_tbl->op_assign(&value3, &value4);

  value3.op_tbl->op_repr(&value2, &value3);
  debug("object_member: %s, type %d", value2.data_ptr, value3.type);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
  value3.op_tbl->op_drop(&value3);

  free(symbols);
}

Test(x86_64_object, test5_assign) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;
  x86_64_value value5;

  uint64_t                    symbols_count = 2;
  x86_64_data_object_symbols *symbols =
      malloc(sizeof(x86_64_data_object_symbols) +
             sizeof(x86_64_data_object_symbol) * symbols_count);

  symbols->count = symbols_count;
  symbols->symbols[0] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"first"};
  symbols->symbols[1] =
      (x86_64_data_object_symbol){.name = (const uint8_t *)"second"};

  __x86_64_proxy_object_init(&value1, symbols);

  // assign
  __x86_64_proxy_void_init(&value4);
  value4.op_tbl->op_assign(&value4, &value1);

  // update something
  __x86_64_proxy_int_init(&value5, 0);

  value1.op_tbl->op_member_ref(&value3, &value1, (const uint8_t *)"second");
  value3.op_tbl->op_assign(&value3, &value5);

  // print each element
  value1.op_tbl->op_repr(&value2, &value1);
  debug("object_assign_1: %s, type %d", value2.data_ptr, value1.type);
  value2.op_tbl->op_drop(&value2);

  value4.op_tbl->op_repr(&value2, &value4);
  debug("object_assign_4: %s, type %d", value2.data_ptr, value4.type);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
  value4.op_tbl->op_drop(&value4);

  free(symbols);
}
