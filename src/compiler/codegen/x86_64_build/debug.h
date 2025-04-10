#pragma once

#include "util/container_util.h"
#include "util/list.h"
#include <stdint.h>

typedef enum cg_debug_level_struct {
  CG_CTX_DEBUG_LEVEL_DISABLED,
  CG_CTX_DEBUG_LEVEL_ENABLED,
} cg_debug_level;

typedef struct cg_debug_param_struct {
  uint64_t    rbp_offset;
  const char *name_ref;
} cg_debug_param;

cg_debug_param *cg_debug_param_new(uint64_t rbp_offset, const char *name_ref);
void            cg_debug_param_free(cg_debug_param *self);

static inline void container_delete_cg_debug_param(void *data) {
  cg_debug_param_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_debug_param, cg_debug_param,
                           container_cmp_false, container_new_move,
                           container_delete_cg_debug_param);

typedef struct cg_debug_var_struct {
  uint64_t    rbp_offset;
  const char *name_ref;
} cg_debug_var;

cg_debug_var *cg_debug_var_new(uint64_t rbp_offset, const char *name_ref);
void          cg_debug_var_free(cg_debug_var *self);

static inline void container_delete_cg_debug_var(void *data) {
  cg_debug_var_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_debug_var, cg_debug_var, container_cmp_false,
                           container_new_move, container_delete_cg_debug_var);

typedef struct cg_debug_sub_struct {
  const char          *sym_start_ref;
  const char          *sym_end_ref;
  const char          *name_ref;
  list_cg_debug_param *params;
  list_cg_debug_var   *vars;
} cg_debug_sub;

cg_debug_sub *cg_debug_sub_new(const char *sym_start_ref,
                               const char *sym_end_ref, const char *name_ref);
void          cg_debug_sub_free(cg_debug_sub *self);

static inline void container_delete_cg_debug_sub(void *data) {
  cg_debug_sub_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_debug_sub, cg_debug_sub, container_cmp_false,
                           container_new_move, container_delete_cg_debug_sub);

typedef struct cg_debug_line_struct {
  const char *sym_address_ref;
  const char *file_ref;
  uint64_t    line;
} cg_debug_line;

cg_debug_line *cg_debug_line_new(const char *sym_address_ref,
                                 const char *file_ref, uint64_t line);
void           cg_debug_line_free(cg_debug_line *self);

static inline void container_delete_cg_debug_line(void *data) {
  cg_debug_line_free(data);
}
LIST_DECLARE_STATIC_INLINE(list_cg_debug_line, cg_debug_line,
                           container_cmp_false, container_new_move,
                           container_delete_cg_debug_line);

typedef struct cg_debug_struct {
  cg_debug_level      level;
  list_cg_debug_sub  *subroutines;
  list_cg_debug_line *lines;
} cg_debug;

cg_debug *cg_debug_new(cg_debug_level level);
void      cg_debug_free(cg_debug *debug);

int cg_debug_enabled(cg_debug *debug);
