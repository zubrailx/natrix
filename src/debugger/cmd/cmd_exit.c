#include "cmd.h"

#include "util/macro.h"

static const char *DESC = "Exit debugger";

static const char *HELP = "Usage: <command>";

static void cmd_exit_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  UNUSED(rest);

  ctx->s_debugger = DEBUGGER_EXITING;
}

cmd *cmd_exit(const char *cmd_ref) {
  return cmd_entry_new(cmd_exit_fn, DESC, HELP, cmd_ref);
}
