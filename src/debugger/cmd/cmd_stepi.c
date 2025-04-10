#include "debugger/cmd/cmd.h"

#include "util/macro.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

static const char *DESC = "Step one instruction exactly";

static const char *HELP = "Usage: <command> [<n>]\n"
                          "\t<n>: number of times";

static void cmd_stepi_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char                   *args = NULL;
  size_t                  n    = 1;
  struct user_regs_struct regs;

  args = trim(strdup(rest));

  if (args && *args) {
    sscanf(args, "%lu", &n);
  }

  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    goto cleanup;
  }

  for (size_t i = 0; i < n; ++i) {
    ctx_target_state status = ctx_tg_stepi(ctx);
    if (status == TARGET_ERROR) {
      ctx_error(ctx, "error while stepping");
    } else if (status == TARGET_NOT_RUNNING) {
      break;
    }
  }

  if (ctx_tg_is_running(ctx)) {
    if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "failed to get regs");
      goto cleanup;
    }
    cmd_disassemble_at_address(ctx, regs.rip, 1);
  }

cleanup:
  if (args) {
    free(args);
  }
}

// 0 - ok, -1 - error
int cmd_stepi_exec(ctx *ctx, uint64_t n) {
  for (size_t i = 0; i < n; ++i) {
    ctx_target_state status = ctx_tg_stepi(ctx);
    if (status == TARGET_ERROR) {
      ctx_error(ctx, "error while stepping");
      return -1;
    } else if (status == TARGET_NOT_RUNNING) {
      break;
    }
  }
  return 0;
}

cmd *cmd_stepi(const char *cmd_ref) {
  return cmd_entry_new(cmd_stepi_fn, DESC, HELP, cmd_ref);
}
