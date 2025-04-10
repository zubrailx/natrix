#pragma once

#include "debugger/ctx/ctx_data.h"
#include "util/strbuf.h"

#include <bfd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/user.h>

// context
typedef struct ctx_struct {
  ctx_debugger_state s_debugger;
  ctx_target_state   s_target;
  strbuf            *buf_out;
  strbuf            *buf_err;
  // internal
  char                   *fname;
  bfd                    *abfd;
  asymbol               **abfd_asymbols;
  int64_t                 abfd_asymbols_count;
  pid_t                   pid;
  hashset_ctx_breakpoint *breaks;
  size_t                  breaks_cnt;
  ctx_debug_data          debug;
  ctx_options             options;
  ctx_debug_line          line; // current line
} ctx;

ctx *ctx_new();
void ctx_free(ctx *self);

void ctx_out_append(ctx *self, const char *data);
void ctx_out_appendln(ctx *self, const char *data);
void ctx_err_append(ctx *self, const char *data);
void ctx_err_appendln(ctx *self, const char *data);

void ctx_error(ctx *ctx, const char *msg);

int              ctx_tg_is_running(ctx *self);
ctx_target_state ctx_tg_get_state(ctx *self);
ctx_target_state ctx_tg_load(ctx *self, uint64_t abfd_asymbols_count,
                             asymbol **abfd_asymbols, bfd *abfd, char *fname);
ctx_target_state ctx_tg_wait(ctx *self);
ctx_target_state ctx_tg_kill(ctx *self, int signal);
ctx_target_state ctx_tg_continue(ctx *self);
ctx_target_state ctx_tg_stepi(ctx *self);
ctx_target_state ctx_tg_peekdata(ctx *self, uint64_t address, uint64_t *data_p);
ctx_target_state ctx_tg_pokedata(ctx *self, uint64_t address, uint64_t data);
ctx_target_state ctx_tg_run(ctx *self, char **argv);
ctx_target_state ctx_tg_getregs(ctx *self, struct user_regs_struct *regs);
ctx_target_state ctx_tg_setregs(ctx *self, struct user_regs_struct *regs);

int ctx_bp_place(ctx *self, ctx_breakpoint *bp);
int ctx_bp_remove(ctx *self, ctx_breakpoint *bp);

void           ctx_dbg_init(ctx *self);
void           ctx_dbg_deinit(ctx *self);
int            ctx_dbg_line_by_source(ctx *self, ctx_debug_line *result);
int            ctx_dbg_line_by_address(ctx *self, ctx_debug_line *result);
const char    *ctx_dbg_file_by_id(ctx *self, uint64_t file);
const char    *ctx_dbg_str_by_id(ctx *self, uint64_t id);
ctx_debug_sub *ctx_dbg_sub_by_address(ctx *self, uint64_t address);

void ctx_dbg_line_reset(ctx *self);
void ctx_dbg_line_set(ctx *self, ctx_debug_line line);
int  ctx_dbg_line_changed(ctx *self, ctx_debug_line line);

#define CTX_WITH_INT3(data) ((data & ~0xFF) | 0xCC)
#define CTX_WITH_STORED(data, stored) ((data & ~0xFF) | stored)
#define CTX_STORE(data) (data & 0xFF)
