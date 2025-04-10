#include "inst.h"
#include <string.h>

static void cg_add_dir_extern_core(cg_ctx *ctx) {
  // io
  cg_ctx_text_push_back(ctx,
                        cg_x86_64_symbol_new_extern(strdup("__x86_64_flush")));
  // make
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_void")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_bool")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_byte")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_int")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_uint")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_long")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_ulong")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_char")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_string")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_callable")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_array")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_object")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_object_setup")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_make_error")));
  // print
  cg_ctx_text_push_back(ctx,
                        cg_x86_64_symbol_new_extern(strdup("__x86_64_print")));
  // unwrap
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_void")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_bool")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_byte")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_int")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_uint")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_long")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_ulong")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_char")));
  cg_ctx_text_push_back(
      ctx, cg_x86_64_symbol_new_extern(strdup("__x86_64_unwrap_string")));
}

void cg_inst_core(cg_ctx *ctx) {
  cg_add_dir_extern_core(ctx);
  return;
}
