#include "data.h"

#include "util/macro.h"

// value_meta
cg_value_meta *cg_value_meta_new(const mir_value *value_ref, int64_t offset,
                                 int is_ptr) {
  cg_value_meta *self = MALLOC(cg_value_meta);
  self->value_ref     = value_ref;
  self->offset        = offset;
  self->is_ptr        = is_ptr;
  return self;
}

void cg_value_meta_free(cg_value_meta *self) {
  if (self) {
    free(self);
  }
}

// mir_sym
cg_mir_sym *cg_mir_sym_new(const void *mir_ref, const char *sym_ref) {
  cg_mir_sym *self = MALLOC(cg_mir_sym);
  self->mir_ref    = mir_ref;
  self->sym_ref    = sym_ref;
  return self;
}

void cg_mir_sym_free(cg_mir_sym *self) {
  if (self) {
    free(self);
  }
}

// type_sym
cg_type_sym *cg_type_sym_new(const void *type_ref, const char *sym_ref) {
  cg_type_sym *self = MALLOC(cg_type_sym);
  self->type_ref    = type_ref;
  self->sym_ref     = sym_ref;
  return self;
}

void cg_type_sym_free(cg_type_sym *self) {
  if (self) {
    free(self);
  }
}

// method_sym
cg_method_sym *cg_method_sym_new(const mir_subroutine *method_ref, char *sym) {
  cg_method_sym *self = MALLOC(cg_method_sym);
  self->method_ref    = method_ref;
  self->sym           = sym;
  return self;
}

void cg_method_sym_free(cg_method_sym *self) {
  if (self) {
    if (self->sym) {
      free(self->sym);
    }
    free(self);
  }
}

// class_sym
cg_class_sym *cg_class_sym_new(const mir_class *class_ref, char *sym) {
  cg_class_sym *self = MALLOC(cg_class_sym);
  self->class_ref    = class_ref;
  self->sym          = sym;
  return self;
}

void cg_class_sym_free(cg_class_sym *self) {
  if (self) {
    if (self->sym) {
      free(self->sym);
    }
    free(self);
  }
}
