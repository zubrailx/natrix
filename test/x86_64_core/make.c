#include <criterion/criterion.h>
#include <stdint.h>

#include "util/log.h"
#include "x86_64_core/builtin/builtin.h"
#include "x86_64_core/proxy/value/int.h"
#include "x86_64_core/value.h"

Test(x86_64_make, test1_array) {
  x86_64_value value1;
  x86_64_value value2;
  x86_64_value value3;
  x86_64_value value4;
  x86_64_value value5;

  __x86_64_make_int(&value1, 3);
  __x86_64_make_int(&value2, 4);
  __x86_64_make_int(&value3, 5);

  __x86_64_make_array(&value4, &value1, &value2, &value3, NULL);

  value4.op_tbl->op_repr(&value5, &value4);
  debug("make_array: %s", value5.data_ptr);
  value5.op_tbl->op_drop(&value5);

  value4.op_tbl->op_drop(&value4);
  value3.op_tbl->op_drop(&value3);
  value2.op_tbl->op_drop(&value2);
  value1.op_tbl->op_drop(&value1);
}

Test(x86_64_make, test2_object) {
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

  __x86_64_make_object(&value1, symbols);

  __x86_64_proxy_int_init(&value4, 0);

  value1.op_tbl->op_member_ref(&value3, &value1, (const uint8_t *)"second");
  value3.op_tbl->op_assign(&value3, &value4);

  value1.op_tbl->op_repr(&value2, &value1);
  debug("make_object: %s, type %d", value2.data_ptr, value1.type);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
  value3.op_tbl->op_drop(&value3);

  free(symbols);
}

Test(x86_64_make, test2_object_setup) {
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

  x86_64_value *defaults = malloc(sizeof(x86_64_value) * symbols_count);

  __x86_64_make_string(defaults + 0, (const uint8_t *)"first_value");
  __x86_64_make_ulong(defaults + 1, UINT64_MAX);

  __x86_64_make_object_setup(&value1, symbols, defaults);

  __x86_64_proxy_int_init(&value4, 0);

  value1.op_tbl->op_member_ref(&value3, &value1, (const uint8_t *)"second");
  value3.op_tbl->op_assign(&value3, &value4);

  value1.op_tbl->op_repr(&value2, &value1);
  debug("make_object: %s, type %d", value2.data_ptr, value1.type);
  value2.op_tbl->op_drop(&value2);

  value1.op_tbl->op_drop(&value1);
  value3.op_tbl->op_drop(&value3);

  free(symbols);
  free(defaults);
}
