#include "cmd.h"

#include "debugger/ctx/str.h"
#include "util/macro.h"
#include "util/string.h"

#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>

static const char *DESC = "Show things about program being debugged";

static const char *HELP = "Usage: <command> "
                          "(sections|symbols|target|registers|breakpoints|"
                          "info|lines|strs|files)";

static char *ctx_debug_idx_str(ctx *ctx, uint64_t str_idx) {
  return ctx->debug.strs[str_idx];
}

static void cmd_info_register_g(ctx *ctx, struct user_regs_struct *regs,
                                const char *reg, size_t offset) {
  char                buf[64];
  unsigned long long *addr = (void *)regs + offset;

  ctx_out_append(ctx, reg);

  ctx_out_append(ctx, "\t");
  snprintf(buf, STRMAXLEN(buf), "0x%-16llx", *addr);
  ctx_out_append(ctx, buf);

  ctx_out_append(ctx, "\t");
  snprintf(buf, STRMAXLEN(buf), "%-16lld", *addr);
  ctx_out_append(ctx, buf);

  ctx_out_appendln(ctx, "");
}

static void cmd_info_sections(ctx *ctx) {
  if (cmd_error_if_no_target(ctx)) {
    return;
  }
  for (asection *section = ctx->abfd->sections; section;
       section           = section->next) {
    char          buf[64];
    const char   *name    = bfd_section_name(section);
    bfd_vma       address = bfd_section_vma(section);
    bfd_size_type size    = bfd_section_size(section);
    ctx_out_append(ctx, "name: ");
    ctx_out_append(ctx, name);
    ctx_out_append(ctx, ",\t");
    ctx_out_append(ctx, "address: ");
    snprintf(buf, STRMAXLEN(buf), "%#lx", address);
    ctx_out_append(ctx, buf);
    ctx_out_append(ctx, ",\t");
    ctx_out_append(ctx, "size: ");
    snprintf(buf, STRMAXLEN(buf), "%#lx", size);
    ctx_out_append(ctx, buf);
    ctx_out_appendln(ctx, " bytes");
  }
}

static void cmd_info_symbols(ctx *ctx) {
  if (cmd_error_if_no_target(ctx)) {
    return;
  }

  for (int64_t i = 0; i < ctx->abfd_asymbols_count; ++i) {
    char        buf[64];
    const char *name         = bfd_asymbol_name(ctx->abfd_asymbols[i]);
    asection   *section      = bfd_asymbol_section(ctx->abfd_asymbols[i]);
    const char *section_name = bfd_section_name(section);
    bfd_vma     address      = bfd_asymbol_value(ctx->abfd_asymbols[i]);
    ctx_out_append(ctx, "section: ");
    ctx_out_append(ctx, section_name);
    ctx_out_append(ctx, ",\t");
    ctx_out_append(ctx, "name: ");
    ctx_out_append(ctx, name);
    ctx_out_append(ctx, ",\t");
    ctx_out_append(ctx, "address: ");
    snprintf(buf, STRMAXLEN(buf), "%#lx", address);
    ctx_out_appendln(ctx, buf);
  }
}

static void cmd_info_target(ctx *ctx) {
  char buf[64];

  if (cmd_error_if_no_target(ctx)) {
    return;
  }

  ctx_out_append(ctx, "file: ");
  ctx_out_append(ctx, ctx->fname);
  ctx_out_append(ctx, ", status: ");
  ctx_out_append(ctx, state_target_enum_str(ctx_tg_get_state(ctx)));

  if (ctx_tg_get_state(ctx) != TARGET_NOT_RUNNING) {
    ctx_out_append(ctx, ", pid: ");
    snprintf(buf, STRMAXLEN(buf), "%d", ctx->pid);
    ctx_out_appendln(ctx, buf);
  } else {
    ctx_out_appendln(ctx, "");
  }
}

static void cmd_info_registers(ctx *ctx) {
  if (cmd_error_if_no_target(ctx) || cmd_error_if_not_running(ctx)) {
    return;
  }

  struct user_regs_struct regs;

  if (ctx_tg_getregs(ctx, &regs) == TARGET_ERROR) {
    ctx_error(ctx, "unable to get registers");
    return;
  }

  cmd_info_register_g(ctx, &regs, "rax",
                      offsetof(struct user_regs_struct, rax));
  cmd_info_register_g(ctx, &regs, "rbx",
                      offsetof(struct user_regs_struct, rbx));
  cmd_info_register_g(ctx, &regs, "rcx",
                      offsetof(struct user_regs_struct, rcx));
  cmd_info_register_g(ctx, &regs, "rdx",
                      offsetof(struct user_regs_struct, rdx));
  cmd_info_register_g(ctx, &regs, "rsi",
                      offsetof(struct user_regs_struct, rsi));
  cmd_info_register_g(ctx, &regs, "rdi",
                      offsetof(struct user_regs_struct, rdi));
  cmd_info_register_g(ctx, &regs, "rbp",
                      offsetof(struct user_regs_struct, rbp));
  cmd_info_register_g(ctx, &regs, "rsp",
                      offsetof(struct user_regs_struct, rsp));
  cmd_info_register_g(ctx, &regs, "r8", offsetof(struct user_regs_struct, r8));
  cmd_info_register_g(ctx, &regs, "r9", offsetof(struct user_regs_struct, r9));
  cmd_info_register_g(ctx, &regs, "r10",
                      offsetof(struct user_regs_struct, r10));
  cmd_info_register_g(ctx, &regs, "r11",
                      offsetof(struct user_regs_struct, r11));
  cmd_info_register_g(ctx, &regs, "r12",
                      offsetof(struct user_regs_struct, r12));
  cmd_info_register_g(ctx, &regs, "r13",
                      offsetof(struct user_regs_struct, r13));
  cmd_info_register_g(ctx, &regs, "r14",
                      offsetof(struct user_regs_struct, r14));
  cmd_info_register_g(ctx, &regs, "r15",
                      offsetof(struct user_regs_struct, r15));
  cmd_info_register_g(ctx, &regs, "rip",
                      offsetof(struct user_regs_struct, rip));
  cmd_info_register_g(ctx, &regs, "eflags",
                      offsetof(struct user_regs_struct, eflags));
  cmd_info_register_g(ctx, &regs, "cs", offsetof(struct user_regs_struct, cs));
  cmd_info_register_g(ctx, &regs, "ss", offsetof(struct user_regs_struct, ss));
  cmd_info_register_g(ctx, &regs, "ds", offsetof(struct user_regs_struct, ds));
  cmd_info_register_g(ctx, &regs, "es", offsetof(struct user_regs_struct, es));
  cmd_info_register_g(ctx, &regs, "fs", offsetof(struct user_regs_struct, fs));
  cmd_info_register_g(ctx, &regs, "gs", offsetof(struct user_regs_struct, gs));
}

static void cmd_info_breakpoints(ctx *ctx) {
  char   buf[64];
  size_t cnt = 0;
  for (hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_begin(ctx->breaks);
       !END(it); NEXT(it)) {
    ++cnt;
  }

  ctx_breakpoint **breakpoints = MALLOCN(ctx_breakpoint *, cnt);
  size_t           i           = 0;
  for (hashset_ctx_breakpoint_it it = hashset_ctx_breakpoint_begin(ctx->breaks);
       !END(it); NEXT(it)) {
    breakpoints[i++] = GET(it);
  }

  if (cnt > 0) {
    for (size_t i = 0; i < cnt - 1; ++i) {
      for (size_t j = 0; j < cnt - i - 1; ++j) {
        if (breakpoints[j]->id > breakpoints[j + 1]->id) {
          ctx_breakpoint *tmp = breakpoints[j];
          breakpoints[j]      = breakpoints[j + 1];
          breakpoints[j + 1]  = tmp;
        }
      }
    }
  }

  for (size_t i = 0; i < cnt; ++i) {
    ctx_breakpoint *b = breakpoints[i];

    ctx_out_append(ctx, "num: ");
    snprintf(buf, STRMAXLEN(buf), "%zu", b->id);
    ctx_out_append(ctx, buf);

    ctx_out_append(ctx, ", address: ");
    snprintf(buf, STRMAXLEN(buf), "0x%016lx", b->address);
    ctx_out_append(ctx, buf);

    ctx_out_append(ctx, ", file: ");
    if (b->file_id != UINT64_MAX) {
      uint64_t strs_idx = ctx->debug.files[b->file_id]->debug_str_id;
      ctx_out_append(ctx, ctx_debug_idx_str(ctx, strs_idx));
    } else {
      ctx_out_append(ctx, "-");
    }

    ctx_out_append(ctx, ", line: ");
    if (b->line != UINT64_MAX) {
      snprintf(buf, STRMAXLEN(buf), "%zu", b->line);
      ctx_out_append(ctx, buf);
    } else {
      ctx_out_append(ctx, "-");
    }

    ctx_out_append(ctx, ", enabled: ");
    if (b->is_enabled) {
      ctx_out_append(ctx, "true");
    } else {
      ctx_out_append(ctx, "false");
    }

    ctx_out_appendln(ctx, "");
  }

  free(breakpoints);
}

static void cmd_info_debug_info(ctx *ctx) {
  if (cmd_error_if_no_target(ctx)) {
    return;
  }
  char buf[64];

  snprintf(buf, STRMAXLEN(buf), "subroutines_cnt: %zu",
           ctx->debug.subroutines_cnt);
  ctx_out_appendln(ctx, buf);

  snprintf(buf, STRMAXLEN(buf), "files_cnt: %zu", ctx->debug.files_cnt);
  ctx_out_appendln(ctx, buf);

  ctx_out_appendln(ctx, "subroutines: ");
  for (uint64_t i = 0; i < ctx->debug.subroutines_cnt; ++i) {
    ctx_debug_sub *sub = ctx->debug.subroutines[i];

    ctx_out_append(ctx, "- name: ");
    ctx_out_appendln(ctx, ctx_debug_idx_str(ctx, sub->debug_str_id));

    snprintf(buf, STRMAXLEN(buf), "\taddress_start: 0x%lx", sub->address_start);
    ctx_out_appendln(ctx, buf);

    snprintf(buf, STRMAXLEN(buf), "\taddress_end: 0x%lx", sub->address_end);
    ctx_out_appendln(ctx, buf);

    snprintf(buf, STRMAXLEN(buf), "\tparams_cnt: %lu", sub->params_cnt);
    ctx_out_appendln(ctx, buf);

    for (uint64_t j = 0; j < sub->params_cnt; ++j) {
      ctx_debug_param arg = sub->params[j];

      snprintf(buf, STRMAXLEN(buf), "\t- name: %s",
               ctx_debug_idx_str(ctx, arg.debug_str_id));
      ctx_out_append(ctx, buf);

      snprintf(buf, STRMAXLEN(buf), ", rbp_offset: %ld", arg.rbp_offset);
      ctx_out_appendln(ctx, buf);
    }

    snprintf(buf, STRMAXLEN(buf), "\tvars_cnt: %lu", sub->vars_cnt);
    ctx_out_appendln(ctx, buf);

    for (uint64_t j = 0; j < sub->vars_cnt; ++j) {
      ctx_debug_var var = sub->vars[j];

      snprintf(buf, STRMAXLEN(buf), "\t- name: %s",
               ctx_debug_idx_str(ctx, var.debug_str_id));
      ctx_out_append(ctx, buf);

      snprintf(buf, STRMAXLEN(buf), ", rbp_offset: %ld", var.rbp_offset);
      ctx_out_appendln(ctx, buf);
    }
  }

  ctx_out_appendln(ctx, "files: ");
  for (uint64_t i = 0; i < ctx->debug.files_cnt; ++i) {
    ctx_debug_file *file = ctx->debug.files[i];
    snprintf(buf, STRMAXLEN(buf), "- id: %lu", i);
    ctx_out_append(ctx, buf);
    ctx_out_append(ctx, ", name: ");
    ctx_out_appendln(ctx, ctx_debug_idx_str(ctx, file->debug_str_id));
  }
}

static void cmd_info_debug_lines(ctx *ctx) {
  if (cmd_error_if_no_target(ctx)) {
    return;
  }
  char buf[64];
  for (uint64_t i = 0; i < ctx->debug.lines_cnt; ++i) {
    ctx_debug_line line = ctx->debug.lines[i];

    snprintf(buf, STRMAXLEN(buf), "%zu: ", i);
    ctx_out_append(ctx, buf);
    cmd_align(ctx, ctx->debug.lines_cnt, i);

    ctx_out_append(ctx, "address: ");
    snprintf(buf, STRMAXLEN(buf), "0x%016lx", line.address);
    ctx_out_append(ctx, buf);

    ctx_out_append(ctx, ", file_id: ");
    snprintf(buf, STRMAXLEN(buf), "%lu", line.file_id);
    ctx_out_append(ctx, buf);

    ctx_out_append(ctx, ", line: ");
    snprintf(buf, STRMAXLEN(buf), "%lu", line.line);
    ctx_out_append(ctx, buf);

    ctx_out_appendln(ctx, "");
  }
}

static void cmd_info_debug_strs(ctx *ctx) {
  if (cmd_error_if_no_target(ctx)) {
    return;
  }
  char buf[64];
  for (uint64_t i = 0; i < ctx->debug.strs_cnt; ++i) {
    snprintf(buf, STRMAXLEN(buf), "%lu: ", i);
    ctx_out_append(ctx, buf);
    ctx_out_appendln(ctx, ctx_debug_idx_str(ctx, i));
  }
}

static void cmd_info_debug_files(ctx *ctx) {
  if (cmd_error_if_no_target(ctx)) {
    return;
  }
  char buf[64];
  for (uint64_t i = 0; i < ctx->debug.files_cnt; ++i) {
    ctx_debug_file *file = ctx->debug.files[i];

    cmd_out_file_header(ctx, i);

    for (uint64_t j = 1; j < file->lines_cnt; ++j) {
      cmd_align(ctx, file->lines_cnt, j);
      snprintf(buf, STRMAXLEN(buf), "%lu", j);
      ctx_out_append(ctx, buf);
      ctx_out_append(ctx, "│");
      ctx_out_append(ctx, file->lines[j]);
    }
  }
}

static void cmd_info_fn(cmd_handler *handler, ctx *ctx, const char *rest) {
  UNUSED(handler);
  char *args;
  args = trim(strdup(rest));

  if (!strcmp(args, "sections")) {
    cmd_info_sections(ctx);
  } else if (!strcmp(args, "symbols")) {
    cmd_info_symbols(ctx);
  } else if (!strcmp(args, "target")) {
    cmd_info_target(ctx);
  } else if (!strcmp(args, "registers")) {
    cmd_info_registers(ctx);
  } else if (!strcmp(args, "breakpoints")) {
    cmd_info_breakpoints(ctx);
  } else if (!strcmp(args, "info")) {
    cmd_info_debug_info(ctx);
  } else if (!strcmp(args, "lines")) {
    cmd_info_debug_lines(ctx);
  } else if (!strcmp(args, "strs")) {
    cmd_info_debug_strs(ctx);
  } else if (!strcmp(args, "files")) {
    cmd_info_debug_files(ctx);
  } else {
    cmd_error_format(ctx, args, HELP);
  }

  if (args) {
    free(args);
  }
}

cmd *cmd_info(const char *cmd_ref) {
  return cmd_entry_new(cmd_info_fn, DESC, HELP, cmd_ref);
}
