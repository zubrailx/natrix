#include "cmd.h"
#include "util/macro.h"

static const char *DESC = "Echo command";

static const char *HELP = "Usage: <command> <arg>...";

static void cmd_echo_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  ctx_out_append(ctx, rest);
}

cmd *cmd_echo(const char *cmd_ref) {
  return cmd_entry_new(cmd_echo_fn, DESC, HELP, cmd_ref);
}
