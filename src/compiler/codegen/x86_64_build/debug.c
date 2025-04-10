#include "debug.h"

#include "util/macro.h"

cg_debug_param *cg_debug_param_new(uint64_t rbp_offset, const char *name_ref) {
  cg_debug_param *self = MALLOC(cg_debug_param);
  self->rbp_offset     = rbp_offset;
  self->name_ref       = name_ref;
  return self;
}

void cg_debug_param_free(cg_debug_param *self) {
  if (self) {
    free(self);
  }
}

cg_debug_var *cg_debug_var_new(uint64_t rbp_offset, const char *name_ref) {
  cg_debug_var *self = MALLOC(cg_debug_var);
  self->rbp_offset   = rbp_offset;
  self->name_ref     = name_ref;
  return self;
}

void cg_debug_var_free(cg_debug_var *self) {
  if (self) {
    free(self);
  }
}

cg_debug_sub *cg_debug_sub_new(const char *sym_start_ref,
                               const char *sym_end_ref, const char *name_ref) {
  cg_debug_sub *self  = MALLOC(cg_debug_sub);
  self->sym_start_ref = sym_start_ref;
  self->sym_end_ref   = sym_end_ref;
  self->name_ref      = name_ref;
  self->params        = list_cg_debug_param_new();
  self->vars          = list_cg_debug_var_new();
  return self;
}

void cg_debug_sub_free(cg_debug_sub *self) {
  if (self) {
    list_cg_debug_param_free(self->params);
    list_cg_debug_var_free(self->vars);
    free(self);
  }
}

cg_debug_line *cg_debug_line_new(const char *sym_address_ref,
                                 const char *file_ref, uint64_t line) {
  cg_debug_line *self   = MALLOC(cg_debug_line);
  self->sym_address_ref = sym_address_ref;
  self->file_ref        = file_ref;
  self->line            = line;
  return self;
}

void cg_debug_line_free(cg_debug_line *self) {
  if (self) {
    free(self);
  }
}

cg_debug *cg_debug_new(cg_debug_level level) {
  cg_debug *self    = MALLOC(cg_debug);
  self->level       = level;
  self->subroutines = list_cg_debug_sub_new();
  self->lines       = list_cg_debug_line_new();
  return self;
}

void cg_debug_free(cg_debug *self) {
  if (self) {
    list_cg_debug_sub_free(self->subroutines);
    list_cg_debug_line_free(self->lines);
    free(self);
  }
}

int cg_debug_enabled(cg_debug *debug) {
  return debug->level == CG_CTX_DEBUG_LEVEL_ENABLED ? 1 : 0;
}
