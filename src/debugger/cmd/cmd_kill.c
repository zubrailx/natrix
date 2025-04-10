#include "cmd.h"

#include "util/macro.h"
#include "util/string.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

static const char *DESC = "Kill running target";

static const char *HELP = "Usage: <command>";

static void cmd_kill_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char *args = NULL;
  int   sig  = SIGKILL;
  char  buf[64];

  args = trim(strdup(rest));
  if (args && *args) {
    sig = atoi(args);
  }

  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    goto cleanup;
  }

  snprintf(buf, STRMAXLEN(buf), "%d", ctx->pid);

  if (ctx_tg_kill(ctx, sig) != TARGET_ERROR) {
    ctx_out_append(ctx, "killed child with pid ");
    ctx_out_appendln(ctx, buf);
  }

cleanup:
  if (args) {
    free(args);
  }
}

cmd *cmd_kill(const char *cmd_ref) {
  return cmd_entry_new(cmd_kill_fn, DESC, HELP, cmd_ref);
}
