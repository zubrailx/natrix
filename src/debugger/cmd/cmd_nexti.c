#include "debugger/cmd/cmd.h"

#include "util/macro.h"
#include "util/string.h"

#include <ctype.h>
#include <dis-asm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

static const char *DESC =
    "Step one instruction, but proceed through subroutine calls";

static const char *HELP = "Usage: <command> [<n>]\n"
                          "\t<n>: number of times";

static int cmd_nexti_fprintf(void *stream, const char *fmt, ...) {
  UNUSED(stream);
  UNUSED(fmt);
  return 0;
}

static int cmd_nexti_fprintf_styled(void *stream, enum disassembler_style style,
                                    const char *fmt, ...) {
  char    buf[64];
  va_list args;

  va_start(args, fmt);

  switch (style) {
    case dis_style_mnemonic:
    case dis_style_sub_mnemonic:
      vsnprintf(buf, STRMAXLEN(buf), fmt, args);
      if (!strncmp(buf, "call", 4) && (isspace(buf[4]) || buf[4] == '\0')) {
        *((int *)stream) = 1;
      }
      break;
    default:
      break;
  }

  va_end(args);

  return 0;
}

static void cmd_nexti_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
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

  {
    disassemble_info   info;
    disassembler_ftype dftype;
    int                is_call;

    cmd_nexti_disasm_init(ctx, &info, &dftype, &is_call);

    if (cmd_nexti_exec(ctx, &info, &dftype, &is_call, n) == -1) {
      goto cleanup;
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

void cmd_nexti_disasm_init(ctx *ctx, disassemble_info *info_p,
                           disassembler_ftype *dftype_p, int *is_call_p) {
  init_disassemble_info(info_p, is_call_p, cmd_nexti_fprintf,
                        cmd_nexti_fprintf_styled);
  info_p->arch              = bfd_arch_i386;
  info_p->mach              = bfd_mach_x86_64;
  info_p->application_data  = ctx;
  info_p->read_memory_func  = cmd_read_memory;
  info_p->memory_error_func = cmd_memory_error;
  *dftype_p = disassembler(bfd_arch_i386, false, bfd_mach_x86_64, NULL);
}

// 0 - ok, -1 - error + outputs to context error
int cmd_nexti_exec(ctx *ctx, disassemble_info *info_p,
                   disassembler_ftype *dftype_p, int *is_call_p, uint64_t n) {
  struct user_regs_struct regs;

  if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
    ctx_error(ctx, "failed to get regs");
    return -1;
  }

  for (size_t i = 0; i < n; ++i) {
    *is_call_p = 0;
    bfd_vma pc = regs.rip;

    size_t insn_size = (*dftype_p)(pc, info_p);
    pc += insn_size;

    if (*is_call_p) {
      ctx_breakpoint *bp = NULL;

      hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_find(
          ctx->breaks, &(ctx_breakpoint){.address = pc});

      if (END(it)) {
        bp = ctx_breakpoint_new(++ctx->breaks_cnt, pc, UINT64_MAX, UINT64_MAX);
        bp->is_enabled = 1;

        hashset_ctx_breakpoint_insert(ctx->breaks, bp);

        ctx_bp_place(ctx, bp);
      }

      if (ctx_tg_continue(ctx) == TARGET_ERROR) {
        ctx_error(ctx, "unable to continue");
        break;
      }

      if (bp) {
        ctx_bp_remove(ctx, bp);
        hashset_ctx_breakpoint_erase(
            ctx->breaks, hashset_ctx_breakpoint_find(ctx->breaks, bp));
        --ctx->breaks_cnt;
      }

    } else {
      if (ctx_tg_stepi(ctx) == TARGET_ERROR) {
        ctx_error(ctx, "unable to stepi");
        break;
      }
    }

    if (!ctx_tg_is_running(ctx)) {
      break;
    }
  }
  return 0;
}

cmd *cmd_nexti(const char *cmd_ref) {
  return cmd_entry_new(cmd_nexti_fn, DESC, HELP, cmd_ref);
}
