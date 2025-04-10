#include "cmd.h"
#include "util/macro.h"
#include "util/string.h"

static const char *DESC = "Configure debugger";

static const char *HELP = "Usage: <command> <arg>...\n"
                          "Options:\n"
                          "* source asm|language - display next command as asm "
                          "instruction or source line";

static int cmd_config_source(ctx *ctx, char *rest) {
  int   status = 0;
  char *arg    = NULL;

  if (!rest || !*rest) {
    status = -1;
    goto cleanup;
  }

  arg = trim(strdup(rest));

  if (!arg) {
    status = -1;
    goto cleanup;
  }

  if (!strcmp(arg, "asm")) {
    ctx->options.source_lang = false;
  } else if (!strcmp(arg, "language")) {
    ctx->options.source_lang = true;
  } else {
    status = -1;
    goto cleanup;
  }

cleanup:
  if (arg) {
    free(arg);
  }
  return status;
}

static void cmd_config_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char *args       = trim(strdup(rest));
  char *arg_option = NULL;
  char *arg_rest   = NULL;
  int   status     = 0;

  {
    char *token_args = strdup(args);
    char *token      = strtok(token_args, " ");
    if (token) {
      arg_option = strdup(token);
      token      = strtok(NULL, "");
    }
    if (token) {
      arg_rest = strdup(token);
    }
    free(token_args);
  }

  if (!arg_option) {
    status = -1;
  }

  if (status != -1) {
    if (!strcmp(arg_option, "source")) {
      status = cmd_config_source(ctx, arg_rest);
    } else {
      status = -1;
    }
  }

  if (status == -1) {
    cmd_error_format(ctx, args, HELP);
  }

  if (arg_rest) {
    free(arg_rest);
  }
  if (arg_option) {
    free(arg_option);
  }
  if (args) {
    free(args);
  }
}

cmd *cmd_config(const char *cmd_ref) {
  return cmd_entry_new(cmd_config_fn, DESC, HELP, cmd_ref);
}
