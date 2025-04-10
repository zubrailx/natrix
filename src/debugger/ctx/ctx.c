#include "ctx.h"

#include "util/macro.h"
#include "util/strbuf.h"
#include <stdlib.h>
#include <sys/wait.h>

ctx *ctx_new() {
  ctx *self        = MALLOC(ctx);
  self->s_debugger = DEBUGGER_IDLE;
  self->s_target   = TARGET_NO_TARGET;
  self->buf_out    = strbuf_new(0, 0);
  self->buf_err    = strbuf_new(0, 0);

  self->fname               = NULL;
  self->abfd                = NULL;
  self->abfd_asymbols       = NULL;
  self->abfd_asymbols_count = 0;
  self->pid                 = 0;
  self->breaks              = hashset_ctx_breakpoint_new();
  self->breaks_cnt          = 0;

  self->debug.subroutines_cnt = 0;
  self->debug.subroutines     = NULL;
  self->debug.files_cnt       = 0;
  self->debug.files           = NULL;
  self->debug.lines_cnt       = 0;
  self->debug.lines           = NULL;
  self->debug.strs_cnt        = 0;
  self->debug.strs            = NULL;

  self->options.source_lang = false;
  return self;
}

void ctx_free(ctx *self) {
  if (self) {
    strbuf_free(self->buf_out);
    strbuf_free(self->buf_err);
    if (self->fname) {
      free(self->fname);
    }
    if (self->abfd_asymbols) {
      free(self->abfd_asymbols);
    }
    if (self->abfd) {
      bfd_close(self->abfd);
    }
    hashset_ctx_breakpoint_free(self->breaks);
    ctx_dbg_deinit(self);
    free(self);
  }
}

void ctx_out_append(ctx *self, const char *data) {
  strbuf_append(self->buf_out, data);
}

void ctx_out_appendln(ctx *self, const char *data) {
  ctx_out_append(self, data);
  ctx_out_append(self, "\n");
}

void ctx_err_append(ctx *self, const char *data) {
  strbuf_append(self->buf_err, data);
}

void ctx_err_appendln(ctx *self, const char *data) {
  ctx_err_append(self, data);
  ctx_err_append(self, "\n");
}

void ctx_error(ctx *ctx, const char *msg) {
  ctx->s_debugger = DEBUGGER_ERROR;
  ctx_err_appendln(ctx, msg);
}
