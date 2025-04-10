#pragma once

#include "compiler/exception/list.h"
#include "compiler/mir/mir.h"

typedef struct mir_merge_bb_result_struct {
  list_exception *exceptions;
} mir_merge_bb_result;

mir_merge_bb_result mir_merge_bb(mir *mir);
