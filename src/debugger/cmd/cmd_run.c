#include "cmd.h"

#include "util/macro.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *DESC = "Run target";

static const char *HELP = "Usage: <command>";

static void cmd_run_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char  *args = NULL;
  char **argv = NULL;

  args = trim(strdup(rest));

  if (cmd_error_if_no_target(ctx)) {
    goto cleanup;
  }

  if (ctx_tg_is_running(ctx)) {
    ctx_error(ctx, "target is already running");
    goto cleanup;
  }

  // parse arguments to command
  size_t size = 4;
  size_t argc = 0;
  argv        = malloc(size * sizeof(char *));
  char *token = strtok(args, " ");
  while (token) {
    if (argc >= size - 1) {
      size *= 2;
      argv = realloc(argv, size * sizeof(char *));
    }
    argv[argc++] = strdup(token);
    token        = strtok(NULL, " ");
  }
  argv[argc] = NULL;

  if (ctx_tg_run(ctx, argv) == TARGET_ERROR) {
    ctx_error(ctx, "target is not created");
    goto cleanup;
  }

  char buf[64];
  ctx_out_append(ctx, "started with pid ");
  snprintf(buf, STRMAXLEN(buf), "%d", ctx->pid);
  ctx_out_appendln(ctx, buf);

  for (hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_begin(ctx->breaks);
       !END(it); NEXT(it)) {
    ctx_breakpoint *bp = GET(it);
    bp->is_placed      = 0;
    ctx_bp_place(ctx, bp);
  }

  ctx_dbg_line_reset(ctx);

cleanup:
  if (argv) {
    for (size_t i = 0; argv[i] != NULL; ++i) {
      free(argv[i]);
    }
    free(argv);
  }
  if (args) {
    free(args);
  }
}

cmd *cmd_run(const char *cmd_ref) {
  return cmd_entry_new(cmd_run_fn, DESC, HELP, cmd_ref);
}
