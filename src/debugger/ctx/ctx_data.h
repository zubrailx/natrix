#pragma once

#include "util/container_util.h"
#include "util/hashset.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// breakpoints
typedef struct ctx_breakpoint_struct {
  size_t   id;
  uint8_t  data;
  uint64_t address;
  uint64_t file_id;
  size_t   line;
  int      is_enabled;
  int      is_placed;
} ctx_breakpoint;

ctx_breakpoint *ctx_breakpoint_new(size_t id, uint64_t address,
                                   uint64_t file_id, size_t line);
void            ctx_breakpoint_free(ctx_breakpoint *self);

static inline int container_cmp_ctx_breakpoint(const void *_lsv,
                                               const void *_rsv) {
  const ctx_breakpoint *lsv = _lsv;
  const ctx_breakpoint *rsv = _rsv;
  int res = container_cmp_uint64((void *)lsv->address, (void *)rsv->address);
  return res;
}
static inline void container_delete_ctx_breakpoint(void *_lsv) {
  return ctx_breakpoint_free(_lsv);
}
static inline uint64_t container_hash_ctx_breakpoint(const void *_lsv) {
  const ctx_breakpoint *lsv = _lsv;
  uint64_t              res = container_hash_uint64((void *)lsv->address);
  return res;
}
HASHSET_DECLARE_STATIC_INLINE(hashset_ctx_breakpoint, ctx_breakpoint,
                              container_cmp_ctx_breakpoint, container_new_move,
                              container_delete_ctx_breakpoint,
                              container_hash_ctx_breakpoint);

// debug
typedef struct ctx_debug_info_param_struct {
  int64_t  rbp_offset;
  uint64_t debug_str_id;
} ctx_debug_param;

typedef struct ctx_debug_var_struct {
  int64_t  rbp_offset;
  uint64_t debug_str_id;
} ctx_debug_var;

typedef struct ctx_debug_sub_struct {
  uint64_t         address_start;
  uint64_t         address_end;
  uint64_t         debug_str_id;
  uint64_t         params_cnt;
  ctx_debug_param *params;
  uint64_t         vars_cnt;
  ctx_debug_var   *vars;
} ctx_debug_sub;

typedef struct ctx_debug_file_struct {
  uint64_t debug_str_id;
  uint64_t lines_cnt;
  char   **lines;
} ctx_debug_file;

typedef struct ctx_debug_line_struct {
  uint64_t address;
  uint64_t file_id;
  uint64_t line;
} ctx_debug_line;

typedef struct ctx_debug_data_struct {
  // info
  uint64_t         subroutines_cnt;
  ctx_debug_sub  **subroutines;
  uint64_t         files_cnt;
  ctx_debug_file **files;
  // line
  uint64_t        lines_cnt;
  ctx_debug_line *lines;
  // str
  uint64_t strs_cnt;
  char   **strs;
} ctx_debug_data;

// state
typedef enum ctx_debugger_state_enum {
  DEBUGGER_UNKNOWN,
  DEBUGGER_IDLE, // waiting for user input
  DEBUGGER_PROCESSING,
  DEBUGGER_ERROR,
  DEBUGGER_EXITING,
} ctx_debugger_state;

typedef enum ctx_target_state_enum {
  TARGET_UNKNOWN,
  TARGET_NO_TARGET,
  TARGET_NOT_RUNNING,
  TARGET_BRKPT,
  TARGET_TRAPPED,
  TARGET_ERROR, // only returned by functions, can't be actual state
} ctx_target_state;

// options
typedef struct ctx_options_struct {
  bool source_lang;
} ctx_options;
