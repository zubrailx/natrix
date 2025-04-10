#pragma once

#include "compiler/type_table/type.h"
#include "compiler/type_table/type_table.h"

const char *type_enum_str(type_enum kind);
char       *type_str(const type_base *generic);
const char *type_mono_enum_str(type_mono_enum mono);

char *type_table_str(const type_table *table, const char *title);
