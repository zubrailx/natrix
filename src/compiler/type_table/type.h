#pragma once

#include "util/container_util.h"
#include "util/list.h"

typedef struct type_entry_struct type_entry;

typedef enum type_enum {
  TYPE_PRIMITIVE = 1,
  TYPE_ARRAY,
  TYPE_CALLABLE,
  TYPE_CLASS_T,
  TYPE_TYPENAME,
  TYPE_MONO,
} type_enum;

typedef enum type_mono_enum {
  TYPE_MONO_UNSET,
  TYPE_MONO_FULL,
  TYPE_MONO_NOT_FULL
} type_mono_enum;

typedef struct type_base_struct {
  type_enum   kind;
  type_entry *type_entry_ref;
} type_base;

// GENERIC
void type_free(type_base *generic);

static inline void container_delete_type(void *data) { type_free(data); }
LIST_DECLARE_STATIC_INLINE(list_type, type_base, container_cmp_false,
                           container_new_move, container_delete_type);

LIST_DECLARE_STATIC_INLINE(list_type_ref, type_base, container_cmp_false,
                           container_new_move, container_delete_false);

void        type_set_type_entry(type_base *generic, type_entry *type_entry_ref);
type_entry *type_get_type_entry(type_base *generic);

type_mono_enum type_calc_mono(const type_base *generic);
type_mono_enum type_get_mono(const type_base *generic);
void           type_set_mono(type_base *generic, type_mono_enum mono);
type_mono_enum type_fetch_mono(type_base *generic);

int            type_cmp(const type_base *lsv, const type_base *rsv);
uint64_t       type_hash(const type_base *lsv);
type_base     *type_copy(const type_base *generic);
list_type_ref *type_copy_list_ref(const list_type_ref *from);

typedef enum type_primitive_enum {
  TYPE_PRIMITIVE_BOOL = 1,
  TYPE_PRIMITIVE_BYTE,
  TYPE_PRIMITIVE_INT,
  TYPE_PRIMITIVE_UINT,
  TYPE_PRIMITIVE_LONG,
  TYPE_PRIMITIVE_ULONG,
  TYPE_PRIMITIVE_CHAR,
  TYPE_PRIMITIVE_STRING,
  TYPE_PRIMITIVE_VOID,
  TYPE_PRIMITIVE_ANY,
} type_primitive_enum;

typedef struct type_primitive_struct {
  type_base           base;
  type_primitive_enum type;
} type_primitive;

type_primitive *type_primitive_new(type_primitive_enum type);

// NOTE:
// * full specialization - all templates match actual types (recursively)
// * partial specialization - (a = C<int, T>) or even (a = C<int, B<T>>)
// * calculation of `spec` is not done inside constructor (cuz it may fail)
//
typedef struct type_array_struct {
  type_base      base;
  type_base     *element_ref;
  type_mono_enum mono_kind;
} type_array;

type_array *type_array_new(type_base *element_ref);

typedef struct type_callable_struct {
  type_base      base;
  type_base     *ret_ref;
  list_type_ref *params;
  type_mono_enum mono_kind;
} type_callable;

type_callable *type_callable_new(type_base *ret_ref, list_type_ref *params);

// primary template
typedef struct type_class_t_struct {
  type_base      base;
  char          *id;
  list_type_ref *typenames;
  list_type_ref *parents;
} type_class_t;

type_class_t *type_class_t_new(char *id, list_type_ref *typenames,
                               list_type_ref *parents);

typedef struct type_typename_struct {
  type_base base;
  // reference to class for ex. which declared this typename
  char      *id;
  type_base *source_ref;
} type_typename;

type_typename *type_typename_new(char *id, type_base *source_ref);

// defines monomorphization of type (not declaration)
// WARN: it should ALWAYS wrap classes

// if type C<A<B>> it will be mono C with mono A with B, so no need to match
// can't be mono of template as class, cuz only support templates as typenames
//
// so something like this in C++ is not allowed:
// template<template <typename> typename T> class A{}; -> T<int> for Ex.
typedef struct type_mono_struct {
  type_base base;
  // list sizes should be the same.
  type_base     *type_ref;
  list_type_ref *types;
  type_mono_enum mono_kind;
} type_mono;

type_mono *type_mono_new(type_base *type_ref, list_type_ref *types);
