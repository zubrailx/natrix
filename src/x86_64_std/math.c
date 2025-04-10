#include "math.h"

#include "util/strbuf.h"
#include "x86_64_core/builtin/builtin.h"

void std_abs(x86_64_value *out, x86_64_value *lsv) {
  switch (lsv->type) {
    case X86_64_TYPE_BYTE: {
      uint8_t l = __x86_64_unwrap_byte(lsv);
      __x86_64_make_byte(out, l);
      break;
    }
    case X86_64_TYPE_INT: {
      int32_t l = __x86_64_unwrap_int(lsv);
      __x86_64_make_int(out, l > 0 ? l : -l);
      break;
    }
    case X86_64_TYPE_LONG: {
      int64_t l = __x86_64_unwrap_long(lsv);
      __x86_64_make_long(out, l > 0 ? l : -l);
      break;
    }
    case X86_64_TYPE_UINT: {
      uint32_t l = __x86_64_unwrap_uint(lsv);
      __x86_64_make_uint(out, l);
      break;
    }
    case X86_64_TYPE_ULONG: {
      uint64_t l = __x86_64_unwrap_ulong(lsv);
      __x86_64_make_ulong(out, l);
      break;
    }
    case X86_64_TYPE_VOID: {
      __x86_64_make_void(out);
      break;
    }
    default: {
      char    buf[64];
      strbuf *buffer = strbuf_new(64, 0);
      strbuf_append(buffer, "incompatible value type '");

      x86_64_value lsv_type;
      lsv->op_tbl->op_type(&lsv_type, lsv);
      strbuf_append(buffer, lsv_type.data_ptr);
      lsv_type.op_tbl->op_drop(&lsv_type);

      strbuf_append_f(buffer, buf, "'(%d) for std_abs", lsv->type);

      x86_64_value error_value;
      __x86_64_make_string_move(&error_value, (uint8_t *)strbuf_detach(buffer));
      __x86_64_make_error(out, lsv);
      break;
    }
  }
}
