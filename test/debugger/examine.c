#include <criterion/criterion.h>
#include <stdint.h>
#include <stdio.h>

#include "util/log.h"
#include "util/macro.h"
#include "util/strbuf.h"

static void print_le_8bytes(strbuf *buffer, uint64_t data, char format,
                            size_t size) {
  char   buf[64];
  size_t offset = 0;

  while (offset != 8) {
    switch (size) {
    case 8: {
      uint64_t value = data;
      switch (format) {
      case 'x':
        snprintf(buf, STRMAXLEN(buf), "0x%lx", value);
        break;
      case 'z':
        snprintf(buf, STRMAXLEN(buf), "0x%016lx", value);
        break;
      case 'd':
        snprintf(buf, STRMAXLEN(buf), "%ld", value);
        break;
      case 'u':
        snprintf(buf, STRMAXLEN(buf), "%lu", value);
        break;
      default:
        warn("unexpected format %c", format);
      }
      break;
    }
    case 4: {
      uint32_t value = data & 0xFFFFFFFF;
      data >>= 32;
      switch (format) {
      case 'x':
        snprintf(buf, STRMAXLEN(buf), "0x%x", value);
        break;
      case 'z':
        snprintf(buf, STRMAXLEN(buf), "0x%08x", value);
        break;
      case 'd':
        snprintf(buf, STRMAXLEN(buf), "%d", value);
        break;
      case 'u':
        snprintf(buf, STRMAXLEN(buf), "%u", value);
        break;
      default:
        warn("unexpected format %c", format);
      }
      break;
    }
    case 2: {
      uint16_t value = data & 0xFFFF;
      data >>= 16;
      switch (format) {
      case 'x':
        snprintf(buf, STRMAXLEN(buf), "0x%hx", value);
        break;
      case 'z':
        snprintf(buf, STRMAXLEN(buf), "0x%04hx", value);
        break;
      case 'd':
        snprintf(buf, STRMAXLEN(buf), "%hd", value);
        break;
      case 'u':
        snprintf(buf, STRMAXLEN(buf), "%hu", value);
        break;
      default:
        warn("unexpected format %c", format);
      }
      break;
    }
    case 1: {
      uint8_t value = data & 0xFF;
      data >>= 8;
      switch (format) {
      case 'x':
        snprintf(buf, STRMAXLEN(buf), "0x%hhx", value);
        break;
      case 'z':
        snprintf(buf, STRMAXLEN(buf), "0x%02hhx", value);
        break;
      case 'd':
        snprintf(buf, STRMAXLEN(buf), "%hhd", value);
        break;
      case 'u':
        snprintf(buf, STRMAXLEN(buf), "%hhu", value);
        break;
      default:
        warn("unexpected format %c", format);
      }
      break;
    }
    default:
      warn("unsupported size");
      return;
    }
    offset += size;
    strbuf_append(buffer, buf);
    strbuf_append(buffer, " ");
  }
}

Test(debugger, examine_format_hex_8) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'x', 8);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_hex_4) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'x', 4);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_hex_2) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'x', 2);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_hex_1) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'x', 1);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_z_8) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'z', 8);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_z_4) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'z', 4);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_z_2) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'z', 2);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_z_1) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'z', 1);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_d_8) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'd', 8);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_d_4) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'd', 4);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_d_2) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'd', 2);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_d_1) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'd', 1);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_u_8) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'u', 8);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_u_4) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'u', 4);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_u_2) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'u', 2);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}

Test(debugger, examine_format_u_1) {
  uint64_t       data = 0xFFF0DEAD;
  strbuf *buffer = strbuf_new(0, 0);

  print_le_8bytes(buffer, data, 'u', 1);

  char *res = strbuf_detach(buffer);
  debug("%s", res);

  free(res);
}
