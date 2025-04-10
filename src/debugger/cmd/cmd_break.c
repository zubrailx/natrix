#include "debugger/cmd/cmd.h"

#include "util/macro.h"
#include "util/string.h"

static const char *DESC = "Set breakpoint at specific location";

static const char *HELP =
    "Usage: <command> (<address>|<file_id> <line>)\n"
    "\t<address>: address in hex\n"
    "\t<file_id>: id of file, see 'info info' or 'info files'\n"
    "\t<line>: line in file";

static void cmd_break_out_breakpoint(ctx *ctx, ctx_breakpoint *breakpoint) {
  char buf[64];
  ctx_out_append(ctx, "breakpoint ");
  snprintf(buf, STRMAXLEN(buf), "%zu ", breakpoint->id);
  ctx_out_append(ctx, buf);
  snprintf(buf, STRMAXLEN(buf), "at 0x%lx: ", breakpoint->address);
  ctx_out_append(ctx, buf);
  ctx_out_append(ctx, "file ");
  if (breakpoint->file_id != UINT64_MAX) {
    ctx_out_append(ctx, ctx_dbg_file_by_id(ctx, breakpoint->file_id));
  } else {
    ctx_out_append(ctx, "-");
  }
  ctx_out_append(ctx, ", line: ");
  if (breakpoint->line != UINT64_MAX) {
    snprintf(buf, STRMAXLEN(buf), "%zu", breakpoint->line);
    ctx_out_append(ctx, buf);
  } else {
    ctx_out_append(ctx, "-");
  }
  ctx_out_appendln(ctx, "");
}

static void cmd_break_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);

  char *args       = NULL;
  char *arg_first  = NULL;
  char *arg_second = NULL;

  args = trim(strdup(rest));

  if (cmd_error_if_no_target(ctx)) {
    goto cleanup;
  }

  {
    char *token_args = strdup(args);
    char *token      = strtok(token_args, " ");
    if (token) {
      arg_first = strdup(token);
      token     = strtok(NULL, " ");
    }
    if (token) {
      arg_second = strdup(token);
      token      = strtok(NULL, " ");
    }
    if (token_args) {
      free(token_args);
    }
  }

  if (!arg_first) {
    goto error_format;
  }

  uint64_t address;
  uint64_t file_id = UINT64_MAX;
  uint64_t line    = UINT64_MAX;

  // breakpoint from file+line
  if (arg_second && *arg_second) {
    ctx_debug_line d_line;
    sscanf(arg_first, "%lu", &file_id);
    sscanf(arg_second, "%lu", &line);
    d_line.file_id = file_id;
    d_line.line    = line;
    if (ctx_dbg_line_by_source(ctx, &d_line) == -1) {
      char buf[64];
      ctx_err_append(ctx, "debug info for file: ");
      ctx_err_append(ctx, ctx_dbg_file_by_id(ctx, file_id));
      ctx_err_append(ctx, buf);
      ctx_err_append(ctx, ", line: ");
      snprintf(buf, STRMAXLEN(buf), "%lu", line);
      ctx_err_append(ctx, buf);
      ctx_error(ctx, " doesn't exist");
      goto cleanup;
    }
    address = d_line.address;
  } else {
    sscanf(arg_first, "%lx", &address);
  }

  hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_find(
      ctx->breaks, &(ctx_breakpoint){.address = address});
  if (!END(it)) {
    char buf[64];
    ctx_err_append(ctx, "breakpoint on address ");
    snprintf(buf, STRMAXLEN(buf), "0x%lx", address);
    ctx_err_append(ctx, buf);
    ctx_error(ctx, " already exists");
    goto cleanup;
  }

  ctx_breakpoint *breakpoint =
      ctx_breakpoint_new(++ctx->breaks_cnt, address, file_id, line);
  breakpoint->is_enabled = 1;

  hashset_ctx_breakpoint_insert(ctx->breaks, breakpoint);

  if (ctx_tg_is_running(ctx)) {
    ctx_bp_place(ctx, breakpoint);
  }

  cmd_break_out_breakpoint(ctx, breakpoint);
  goto cleanup;

error_format:
  cmd_error_format(ctx, args, HELP);

cleanup:
  if (arg_first) {
    free(arg_first);
  }
  if (arg_second) {
    free(arg_second);
  }
  if (args) {
    free(args);
  }
}

cmd *cmd_break(const char *cmd_ref) {
  return cmd_entry_new(cmd_break_fn, DESC, HELP, cmd_ref);
}
