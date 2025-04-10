#include "debugger/cmd/cmd.h"

#include "util/macro.h"
#include "util/string.h"

static const char *DESC = "Delete breakpoint by id or all";

static const char *HELP = "Usage: <command> [<id>]\n"
                          "\t<id>: id of breakpoint";

static void cmd_delete_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char *args   = NULL;
  char *arg_id = NULL;

  args = trim(strdup(rest));

  if (cmd_error_if_no_target(ctx)) {
    goto cleanup;
  }

  {
    char *token_args = strdup(args);
    char *token      = strtok(token_args, " ");
    if (token) {
      arg_id = strdup(token);
      token  = strtok(NULL, " ");
    }
    if (token_args) {
      free(token_args);
    }
  }

  uint64_t id;
  if (!arg_id || !*arg_id) {
    id = 0;
  } else {
    sscanf(arg_id, "%lu", &id);
  }

  size_t deleted = 0;

  for (hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_begin(ctx->breaks);
       !END(it); NEXT(it)) {

    ctx_breakpoint *b = GET(it);

    if (!id || b->id == id) {
      ++deleted;
      if (ctx_tg_is_running(ctx)) {
        ctx_bp_remove(ctx, b);
      }
      if (id) {
        hashset_ctx_breakpoint_erase(ctx->breaks, it);
        break;
      }
    }
  }

  if (!id) {
    hashset_ctx_breakpoint_free(ctx->breaks);
    ctx->breaks = hashset_ctx_breakpoint_new();
  }

  if (!deleted) {
    ctx_error(ctx, "no breakpoints match");
    goto cleanup;
  }

cleanup:
  if (arg_id) {
    free(arg_id);
  }
  if (args) {
    free(args);
  }
}

cmd *cmd_delete(const char *cmd_ref) {
  return cmd_entry_new(cmd_delete_fn, DESC, HELP, cmd_ref);
}
