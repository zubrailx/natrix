#include "debugger/cmd/cmd.h"

#include "util/macro.h"
#include "util/string.h"

#include <dis-asm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

static const char *DESC =
    "Step one source line, but proceed through subroutine calls";

static const char *HELP = "Usage: <command> [<n>]\n"
                          "\t<n>: number of times";

static void cmd_next_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char                   *args = NULL;
  size_t                  n    = 1;
  struct user_regs_struct regs;
  ctx_debug_line          curr;

  args = trim(strdup(rest));

  if (args && *args) {
    sscanf(args, "%lu", &n);
  }

  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    goto cleanup;
  }

  // get current address
  if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    goto cleanup;
  }

  curr = ctx->line;

  disassemble_info   info;
  disassembler_ftype dftype;
  int                is_call;
  cmd_nexti_disasm_init(ctx, &info, &dftype, &is_call);

  for (size_t i = 0; i < n; ++i) {
    while (!ctx_dbg_line_changed(ctx, curr)) {
      if (cmd_nexti_exec(ctx, &info, &dftype, &is_call, n) == -1) {
        goto cleanup;
      }

      if (!ctx_tg_is_running(ctx)) {
        break;
      }

      if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
        ctx_error(ctx, "failed to get regs");
        goto cleanup;
      }

      curr.address = regs.rip;
      ctx_dbg_line_by_address(ctx, &curr);
    }
    ctx_dbg_line_set(ctx, curr);
  }

  if (ctx_tg_is_running(ctx)) {
    cmd_out_line(ctx, curr);
  }

cleanup:
  if (args) {
    free(args);
  }
}

cmd *cmd_next(const char *cmd_ref) {
  return cmd_entry_new(cmd_next_fn, DESC, HELP, cmd_ref);
}
