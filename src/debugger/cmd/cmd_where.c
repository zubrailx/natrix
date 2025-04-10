#include "cmd.h"
#include "util/macro.h"

static const char *DESC = "Print stacktrace";

static const char *HELP = "Usage: <command>";

static uint64_t cmd_where_p_rip(uint64_t rbp) { return rbp + 8; }
static uint64_t cmd_where_p_rbp(uint64_t rbp) { return rbp; }

// TODO: check current instruction if in prologue/epilogue
static void cmd_where_traverse(ctx *ctx, uint64_t rbp, uint64_t rip) {
  char buf[64];
  // reaches top (also can check if top main reached)
  if (rbp == 1) {
    return;
  }

  ctx_debug_sub *sub = ctx_dbg_sub_by_address(ctx, rip);
  if (!sub) {
    ctx_error(ctx, "no debug info about current subroutine");
    return;
  }

  snprintf(buf, STRMAXLEN(buf), "0x%016lx: ", rip);
  ctx_out_append(ctx, buf);

  ctx_out_append(ctx, ctx_dbg_str_by_id(ctx, sub->debug_str_id));

  ctx_out_append(ctx, " (");
  for (uint64_t i = 0; i < sub->params_cnt; ++i) {
    if (i) {
      ctx_out_append(ctx, ", ");
    }

    ctx_debug_param *param      = sub->params + i;
    const char      *param_name = ctx_dbg_str_by_id(ctx, param->debug_str_id);
    uint64_t         address    = rbp + param->rbp_offset;

    char *data = cmd_print_value_data(ctx, address);
    char *type = cmd_print_value_type(ctx, address);

    cmd_out_value(ctx, param_name, data, type);
  }
  ctx_out_appendln(ctx, ")");

  uint64_t p_rip;
  if (ctx_tg_peekdata(ctx, cmd_where_p_rip(rbp), &p_rip) == TARGET_ERROR) {
    ctx_error(ctx, "unable to peek data");
    return;
  }

  uint64_t p_rbp;
  if (ctx_tg_peekdata(ctx, cmd_where_p_rbp(rbp), &p_rbp) == TARGET_ERROR) {
    ctx_error(ctx, "unable to peek data");
    return;
  }

  cmd_where_traverse(ctx, p_rbp, p_rip);
}

static void cmd_where_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  UNUSED(rest);
  struct user_regs_struct regs;

  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    goto cleanup;
  }

  if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    goto cleanup;
  }

  cmd_where_traverse(ctx, regs.rbp, regs.rip);

cleanup:
  return;
}

cmd *cmd_where(const char *cmd_ref) {
  return cmd_entry_new(cmd_where_fn, DESC, HELP, cmd_ref);
}
