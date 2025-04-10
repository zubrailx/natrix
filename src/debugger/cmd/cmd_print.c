#include "cmd.h"

#include "util/macro.h"
#include "util/string.h"
#include "x86_64_core/value.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <unistd.h>

static const char *DESC = "Print register, argument or variable";

static const char *HELP = "Usage: <command> (<register>|<arg>|<var>)\n"
                          "\t<register>: $<name>\n"
                          "\t<arg>: argument name\n"
                          "\t<var>: variable name";

static int cmd_print_register(ctx *ctx, const char *rest) {
  char                    buf[64];
  struct user_regs_struct regs;
  const char             *reg = rest + 1;

  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    return -1;
  }

  if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    return -1;
  }

  size_t offset = SIZE_MAX;

  if (!(strcmp(reg, "rax"))) {
    offset = offsetof(struct user_regs_struct, rax);
  } else if (!(strcmp(reg, "rbx"))) {
    offset = offsetof(struct user_regs_struct, rbx);
  } else if (!(strcmp(reg, "rcx"))) {
    offset = offsetof(struct user_regs_struct, rcx);
  } else if (!(strcmp(reg, "rdx"))) {
    offset = offsetof(struct user_regs_struct, rdx);
  } else if (!(strcmp(reg, "rsi"))) {
    offset = offsetof(struct user_regs_struct, rsi);
  } else if (!(strcmp(reg, "rdi"))) {
    offset = offsetof(struct user_regs_struct, rdi);
  } else if (!(strcmp(reg, "rbp"))) {
    offset = offsetof(struct user_regs_struct, rbp);
  } else if (!(strcmp(reg, "rsp"))) {
    offset = offsetof(struct user_regs_struct, rsp);
  } else if (!(strcmp(reg, "r8"))) {
    offset = offsetof(struct user_regs_struct, r8);
  } else if (!(strcmp(reg, "r9"))) {
    offset = offsetof(struct user_regs_struct, r9);
  } else if (!(strcmp(reg, "r10"))) {
    offset = offsetof(struct user_regs_struct, r10);
  } else if (!(strcmp(reg, "r11"))) {
    offset = offsetof(struct user_regs_struct, r11);
  } else if (!(strcmp(reg, "r12"))) {
    offset = offsetof(struct user_regs_struct, r12);
  } else if (!(strcmp(reg, "r13"))) {
    offset = offsetof(struct user_regs_struct, r13);
  } else if (!(strcmp(reg, "r14"))) {
    offset = offsetof(struct user_regs_struct, r14);
  } else if (!(strcmp(reg, "r15"))) {
    offset = offsetof(struct user_regs_struct, r15);
  } else if (!(strcmp(reg, "rip"))) {
    offset = offsetof(struct user_regs_struct, rip);
  } else if (!(strcmp(reg, "eflags"))) {
    offset = offsetof(struct user_regs_struct, eflags);
  } else if (!(strcmp(reg, "cs"))) {
    offset = offsetof(struct user_regs_struct, cs);
  } else if (!(strcmp(reg, "ss"))) {
    offset = offsetof(struct user_regs_struct, ss);
  } else if (!(strcmp(reg, "ds"))) {
    offset = offsetof(struct user_regs_struct, ds);
  } else if (!(strcmp(reg, "es"))) {
    offset = offsetof(struct user_regs_struct, es);
  } else if (!(strcmp(reg, "fs"))) {
    offset = offsetof(struct user_regs_struct, fs);
  } else if (!(strcmp(reg, "gs"))) {
    offset = offsetof(struct user_regs_struct, gs);
  }

  if (offset == SIZE_MAX) {
    ctx->s_debugger = DEBUGGER_ERROR;
    ctx_err_append(ctx, "unknown register ");
    ctx_err_appendln(ctx, reg);
    return -1;
  }

  snprintf(buf, STRMAXLEN(buf), "%#llx",
           *(unsigned long long *)(((void *)&regs) + offset));
  ctx_out_appendln(ctx, buf);
  return 0;
}

static int cmd_print_param_var(ctx *ctx, const char *rest) {
  const char             *args = rest;
  struct user_regs_struct regs;

  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    return -1;
  }

  if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    return -1;
  }

  // get current subroutine based on rsp
  ctx_debug_sub *sub = ctx_dbg_sub_by_address(ctx, regs.rip);
  if (!sub) {
    ctx_error(ctx, "no debug info about current subroutine");
    return -1;
  }

  int found = 0;

  for (uint64_t i = 0; i < sub->params_cnt; ++i) {
    const ctx_debug_param *param = sub->params + i;
    const char *param_name       = ctx_dbg_str_by_id(ctx, param->debug_str_id);
    if (!strcmp(param_name, args)) {
      uint64_t address = regs.rbp + param->rbp_offset;
      char    *data    = cmd_print_value_data(ctx, address);
      char    *type    = cmd_print_value_type(ctx, address);

      cmd_out_value(ctx, param_name, data, type);
      ctx_out_appendln(ctx, "");

      found = 1;
    }
  }

  for (uint64_t i = 0; i < sub->vars_cnt; ++i) {
    const ctx_debug_var *var      = sub->vars + i;
    const char          *var_name = ctx_dbg_str_by_id(ctx, var->debug_str_id);
    if (!strcmp(var_name, args)) {
      uint64_t address = regs.rbp + var->rbp_offset;
      char    *data    = cmd_print_value_data(ctx, address);
      char    *type    = cmd_print_value_type(ctx, address);

      cmd_out_value(ctx, var_name, data, type);
      ctx_out_appendln(ctx, "");

      found = 1;
    }
  }

  if (!found) {
    ctx_err_append(ctx, "in subroutine ");
    ctx_err_append(ctx, ctx_dbg_str_by_id(ctx, sub->debug_str_id));
    ctx_error(ctx, " arg or var not found");
    return -1;
  }

  return 0;
}

static void cmd_print_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);

  char *args = trim(strdup(rest));

  if (!args || !*args) {
    goto error_format;
  }

  if (args[0] == '$') {
    cmd_print_register(ctx, args);
  } else {
    cmd_print_param_var(ctx, args);
  }
  goto cleanup;

error_format:
  cmd_error_format(ctx, args, HELP);

cleanup:
  if (args) {
    free(args);
  }
}

static uint64_t cmd_print_aligned(uint64_t address) {
  return address - (address % 16);
}

// returns reference to created breakpoint, don't delete
static ctx_breakpoint *cmd_print_bp_place(ctx *ctx, uint64_t address) {
  ctx_breakpoint *bp = NULL;

  hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_find(
      ctx->breaks, &(ctx_breakpoint){.address = address});

  if (END(it)) {
    bp = ctx_breakpoint_new(ctx->breaks_cnt++, address, UINT64_MAX, UINT64_MAX);
    bp->is_enabled = 1;

    hashset_ctx_breakpoint_insert(ctx->breaks, bp);
    ctx_bp_place(ctx, bp);
  }

  return bp;
}

static void cmd_print_bp_delete(ctx *ctx, ctx_breakpoint *bp) {
  if (bp) {
    ctx_bp_remove(ctx, bp);
    hashset_ctx_breakpoint_erase(ctx->breaks,
                                 hashset_ctx_breakpoint_find(ctx->breaks, bp));
    --ctx->breaks_cnt;
  }
}

// get func address of op_repr
static int cmd_print_op_tbl_func(ctx *ctx, uint64_t address, uint64_t offset,
                                 uint64_t *addr_func_out) {
  if (ctx_tg_peekdata(ctx, address + offsetof(x86_64_value, op_tbl),
                      addr_func_out) == TARGET_ERROR) {
    ctx_error(ctx, "unable to peek data(op_tbl value)");
    return -1;
  }

  if (ctx_tg_peekdata(ctx, *addr_func_out + offset, addr_func_out) ==
      TARGET_ERROR) {
    ctx_error(ctx, "unable to peek data(op_tbl value)");
    return -1;
  }

  return 0;
}

// call function and check whether rbp matches old rbp
static int cmd_print_call(ctx *ctx, const struct user_regs_struct *regs_old) {
  struct user_regs_struct regs;
  int                     is_inside   = 0;
  int                     is_other_bp = 0;

  do {
    switch (ctx_tg_continue(ctx)) {
      case TARGET_BRKPT:
      case TARGET_TRAPPED: {
        if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
          ctx_error(ctx, "unable to get regs while handling breakpoints");
          return -1;
        }
        is_inside   = regs.rbp != regs_old->rbp;
        is_other_bp = regs.rip != regs_old->rip;
        break;
      }
      case TARGET_UNKNOWN:
      case TARGET_NO_TARGET:
      case TARGET_NOT_RUNNING:
      case TARGET_ERROR:
        is_inside   = 0;
        is_other_bp = 0;
        break;
    }

  } while (is_inside || is_other_bp);

  return 0;
}

static char *cmd_print_string(ctx *ctx, uint64_t addr_value) {
  uint64_t data;
  int32_t  type;

  if (ctx_tg_peekdata(ctx, addr_value, &data) == TARGET_ERROR) {
    ctx_error(ctx, "unable to fetch result type");
    return NULL;
  }

  type = data;

  if (type != X86_64_TYPE_STRING) {
    char buf[64];
    snprintf(buf, STRMAXLEN(buf), "op_repr returned type %d instead of %d(%s)",
             type, X86_64_TYPE_STRING, "string");
    ctx_error(ctx, buf);
    return NULL;
  }

  if (ctx_tg_peekdata(ctx, addr_value + offsetof(x86_64_value, data_ptr),
                      &data) == TARGET_ERROR) {
    ctx_error(ctx, "unable to fetch result string");
    return NULL;
  }

  // copy string to local memory
  strbuf *buffer = strbuf_new(0, 0);

  char bytes[9];
  bytes[8] = '\0';

  for (uint64_t offset = 0; offset != UINT64_MAX; offset += 8) {
    if (ctx_tg_peekdata(ctx, data + offset, (uint64_t *)bytes) ==
        TARGET_ERROR) {
      ctx_error(ctx, "unable to fetch result string");
      strbuf_free(buffer);
      return NULL;
    }

    strbuf_append(buffer, bytes);

    // check whether reached the end of string
    for (uint64_t i = 0; i < 8; ++i) {
      if (bytes[i] == 0) {
        offset = UINT64_MAX - 8;
        break;
      }
    }
  }

  return strbuf_detach(buffer);
}

char *cmd_print_value_data(ctx *ctx, uint64_t address) {
  struct user_regs_struct regs_old;
  struct user_regs_struct regs;
  ctx_breakpoint         *bp     = NULL;
  char                   *result = NULL;

  if (ctx_tg_getregs(ctx, &regs_old) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    goto cleanup;
  }

  regs = regs_old;

  // place breakpoint under current instruction of no breakpoints present
  bp = cmd_print_bp_place(ctx, regs.rip);

  // call op_repr
  {
    uint64_t addr_func;

    if (cmd_print_op_tbl_func(ctx, address, offsetof(x86_64_op_tbl, op_repr),
                              &addr_func)) {
      goto cleanup;
    }

    regs.rsp -= sizeof(x86_64_value);
    regs.rsp = cmd_print_aligned(regs.rsp);

    regs.rdi = regs.rsp;
    regs.rsi = address;

    regs.rsp -= 8;
    if (ctx_tg_pokedata(ctx, regs.rsp, regs.rip) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set return address after op_repr call");
      goto cleanup;
    }
    regs.rip = addr_func;

    if (ctx_tg_setregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set regs(with op_repr)");
      goto cleanup;
    }

    if (cmd_print_call(ctx, &regs_old)) {
      goto cleanup;
    }
  }

  // retrieve result
  {
    if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "unable to get regs after calling op_repr");
      goto cleanup;
    }

    result = cmd_print_string(ctx, regs.rsp);
    if (!result) {
      goto cleanup;
    }
  }

  // call destructor on result
  {
    uint64_t addr_drop;

    if (cmd_print_op_tbl_func(ctx, regs.rsp, offsetof(x86_64_op_tbl, op_drop),
                              &addr_drop)) {
      goto cleanup;
    };

    regs.rdi = regs.rsp;

    regs.rsp -= 8;
    if (ctx_tg_pokedata(ctx, regs.rsp, regs.rip) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set return address after op_drop call");
      goto cleanup;
    }
    regs.rip = addr_drop;

    if (ctx_tg_setregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set regs(with op_repr)");
      goto cleanup;
    }

    if (cmd_print_call(ctx, &regs_old)) {
      goto cleanup;
    }
  }

cleanup:
  cmd_print_bp_delete(ctx, bp);

  // restore registers
  if (ctx_tg_setregs(ctx, &regs_old) == TARGET_ERROR) {
    ctx_error(ctx, "unable to restore regs");
  }

  return result;
}

char *cmd_print_value_type(ctx *ctx, uint64_t address) {
  struct user_regs_struct regs_old;
  struct user_regs_struct regs;
  ctx_breakpoint         *bp     = NULL;
  char                   *result = NULL;

  if (ctx_tg_getregs(ctx, &regs_old) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    goto cleanup;
  }

  regs = regs_old;

  // place breakpoint under current instruction of no breakpoints present
  bp = cmd_print_bp_place(ctx, regs.rip);

  // call op_type
  {
    uint64_t addr_func;

    if (cmd_print_op_tbl_func(ctx, address, offsetof(x86_64_op_tbl, op_type),
                              &addr_func)) {
      goto cleanup;
    }

    regs.rsp -= sizeof(x86_64_value);
    regs.rsp = cmd_print_aligned(regs.rsp);

    regs.rdi = regs.rsp;
    regs.rsi = address;

    regs.rsp -= 8;
    if (ctx_tg_pokedata(ctx, regs.rsp, regs.rip) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set return address after op_type call");
      goto cleanup;
    }
    regs.rip = addr_func;

    if (ctx_tg_setregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set regs(with op_repr)");
      goto cleanup;
    }

    if (cmd_print_call(ctx, &regs_old)) {
      goto cleanup;
    }
  }

  // retrieve result
  {
    if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "unable to get regs after calling op_type");
      goto cleanup;
    }

    result = cmd_print_string(ctx, regs.rsp);
    if (!result) {
      goto cleanup;
    }
  }

  // call destructor on result
  {
    uint64_t addr_drop;

    if (cmd_print_op_tbl_func(ctx, regs.rsp, offsetof(x86_64_op_tbl, op_drop),
                              &addr_drop)) {
      goto cleanup;
    };

    regs.rdi = regs.rsp;

    regs.rsp -= 8;
    if (ctx_tg_pokedata(ctx, regs.rsp, regs.rip) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set return address after op_drop call");
      goto cleanup;
    }
    regs.rip = addr_drop;

    if (ctx_tg_setregs(ctx, &regs) == TARGET_ERROR) {
      ctx_error(ctx, "unable to set regs(with op_repr)");
      goto cleanup;
    }

    if (cmd_print_call(ctx, &regs_old)) {
      goto cleanup;
    }
  }

cleanup:
  cmd_print_bp_delete(ctx, bp);

  // restore registers
  if (ctx_tg_setregs(ctx, &regs_old) == TARGET_ERROR) {
    ctx_error(ctx, "unable to restore regs");
  }

  return result;
}

cmd *cmd_print(const char *cmd_ref) {
  return cmd_entry_new(cmd_print_fn, DESC, HELP, cmd_ref);
}
