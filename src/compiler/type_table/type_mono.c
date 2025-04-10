#include "type.h"

#include "util/log.h"
#include "util/macro.h"

static type_mono_enum type_calc_mono_list(list_type_ref *list) {
  if (!list) {
    return TYPE_MONO_UNSET;
  }
  for (list_type_ref_it it = list_type_ref_begin(list); !END(it); NEXT(it)) {
    type_mono_enum mono = type_calc_mono(GET(it));
    if (mono == TYPE_MONO_UNSET || mono == TYPE_MONO_NOT_FULL) {
      return mono;
    }
  }
  return TYPE_MONO_FULL;
}

type_mono_enum type_calc_mono(const type_base *generic) {
  if (!generic) {
    return TYPE_MONO_FULL;
  }

  switch (generic->kind) {

    case TYPE_PRIMITIVE:
      return TYPE_MONO_FULL;

    case TYPE_ARRAY: {
      const type_array *self = (const type_array *)generic;
      return type_calc_mono(self->element_ref);
    }

    case TYPE_CALLABLE: {
      const type_callable *self = (const type_callable *)generic;
      type_mono_enum       mono = type_calc_mono(self->ret_ref);
      if (mono == TYPE_MONO_UNSET || mono == TYPE_MONO_NOT_FULL) {
        return mono;
      }
      return type_calc_mono_list(self->params);
    }

    case TYPE_CLASS_T: {
      const type_class_t *self = (const type_class_t *)generic;
      if (!self->typenames) {
        return TYPE_MONO_UNSET;
      }
      if (list_type_ref_size(self->typenames)) {
        return TYPE_MONO_NOT_FULL;
      }
      return type_calc_mono_list(self->parents);
    }

    case TYPE_TYPENAME:
      return TYPE_MONO_NOT_FULL;

    case TYPE_MONO: {
      const type_mono *self = (const type_mono *)generic;

      if (!self->type_ref) {
        error("type_mono isn't attached to any type (null)");
        return TYPE_MONO_UNSET;
      }

      switch (self->type_ref->kind) {
        case TYPE_CLASS_T: {
          const type_class_t *attached = (const type_class_t *)self->type_ref;

          size_t typenames_sz = list_type_ref_size(attached->typenames);
          size_t types_sz     = list_type_ref_size(self->types);
          if (typenames_sz != types_sz) {
            error("typenames size is not equal to types size");
            return TYPE_MONO_UNSET;
          }

          return type_calc_mono_list(self->types);
        }
        default:
          error("type_mono currently can be attached ONLY to classes");
          return TYPE_MONO_UNSET;
      }
    }
  }
  error("unexpected type kind %d %p", generic->kind, generic);
  return TYPE_MONO_UNSET;
}

type_mono_enum type_get_mono(const type_base *generic) {
  if (!generic) {
    return TYPE_MONO_FULL;
  }
  switch (generic->kind) {
    case TYPE_PRIMITIVE:
      return TYPE_MONO_FULL;
    case TYPE_TYPENAME:
      return TYPE_MONO_NOT_FULL;
    case TYPE_ARRAY:
      return ((const type_array *)generic)->mono_kind;
    case TYPE_CALLABLE:
      return ((const type_callable *)generic)->mono_kind;
    case TYPE_CLASS_T:
      return TYPE_MONO_NOT_FULL;
    case TYPE_MONO:
      return ((const type_mono *)generic)->mono_kind;
  }
  error("unexpected type kind %d %p", generic->kind, generic);
  return TYPE_MONO_UNSET;
}

void type_set_mono(type_base *generic, type_mono_enum mono) {
  if (!generic) {
    return;
  }
  switch (generic->kind) {
    case TYPE_PRIMITIVE:
      break;
    case TYPE_ARRAY:
      ((type_array *)generic)->mono_kind = mono;
      break;
    case TYPE_CALLABLE:
      ((type_callable *)generic)->mono_kind = mono;
      break;
    case TYPE_CLASS_T:
      break;
    case TYPE_TYPENAME:
      break;
    case TYPE_MONO:
      ((type_mono *)generic)->mono_kind = mono;
      break;
    default:
      error("unexpected type kind %d %p", generic->kind, generic);
      break;
  }
}

type_mono_enum type_fetch_mono(type_base *generic) {
  if (!generic) {
    return TYPE_MONO_FULL;
  }
  type_mono_enum mono;
  if ((mono = type_get_mono(generic)) == TYPE_MONO_UNSET) {
    mono = type_calc_mono(generic);
    type_set_mono(generic, mono);
  }
  return mono;
}
